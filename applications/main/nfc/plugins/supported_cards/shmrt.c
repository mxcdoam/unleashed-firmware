#include "nfc_supported_card_plugin.h"
#include <flipper_application.h>
#include <nfc/protocols/fudan_fm11rf005/fudan_fm11rf005.h>
#include <flipper_format/flipper_format.h>
#include <datetime.h>

#define TAG "Shmrt"

#define SHMRT_PAGE_FLAGS 2U
#define SHMRT_PAGE_STA   5U
#define SHMRT_PAGE_TS    6U
#define SHMRT_PAGE_FARE  7U

#define SHMRT_FLAGS_1J   0x0002U
#define SHMRT_FLAGS_PASS 0x0290U

#define SHMRT_TYPE_1J   0x64U
#define SHMRT_TYPE_1DAY 0x84U
#define SHMRT_TYPE_3DAY 0x88U

#define SHMRT_TS_NONE 0xFFFFFFFFU

static bool shmrt_verify(Nfc* nfc) {
    UNUSED(nfc);
    return true;
}

static bool shmrt_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    furi_assert(parsed_data);

    const FudanFm11rf005Data* data = nfc_device_get_data(device, NfcProtocolFudanFm11rf005);
    bool parsed = false;

    do {
        uint16_t flags = (uint16_t)(data->pages[SHMRT_PAGE_FLAGS][0] << 8) |
                         data->pages[SHMRT_PAGE_FLAGS][1];
        if(flags != SHMRT_FLAGS_1J && flags != SHMRT_FLAGS_PASS) break;

        uint8_t type = data->pages[SHMRT_PAGE_STA][3];
        const char* type_str;
        if(type == SHMRT_TYPE_1J) {
            type_str = "Single journey";
        } else if(type == SHMRT_TYPE_1DAY) {
            type_str = "1 day pass";
        } else if(type == SHMRT_TYPE_3DAY) {
            type_str = "3 days pass";
        } else {
            break;
        }

        furi_string_printf(parsed_data, "\e#Shanghai Metro\n");
        furi_string_cat_printf(parsed_data, "Type: %s\n", type_str);

        if(type == SHMRT_TYPE_1J) {
            uint16_t fare = (uint16_t)(data->pages[SHMRT_PAGE_FARE][0] << 8) |
                            data->pages[SHMRT_PAGE_FARE][1];
            furi_string_cat_printf(parsed_data, "Fare: %u.%02u CNY\n", fare / 100, fare % 100);

            uint32_t ts = (uint32_t)data->pages[SHMRT_PAGE_TS][0] << 24 |
                          (uint32_t)data->pages[SHMRT_PAGE_TS][1] << 16 |
                          (uint32_t)data->pages[SHMRT_PAGE_TS][2] << 8 |
                          (uint32_t)data->pages[SHMRT_PAGE_TS][3];

            if(ts != SHMRT_TS_NONE) {
                DateTime dt;
                datetime_timestamp_to_datetime(ts, &dt);

                furi_string_cat_printf(
                    parsed_data, "Issued on: %02u-%02u-%04u\n", dt.day, dt.month, dt.year);
                furi_string_cat_printf(
                    parsed_data, "Issue time: %02u:%02u:%02u\n", dt.hour, dt.minute, dt.second);
            }
        }

        uint16_t sta_id = (uint16_t)(data->pages[SHMRT_PAGE_STA][0] << 8) |
                          data->pages[SHMRT_PAGE_STA][1];

        uint8_t line_bcd = data->pages[SHMRT_PAGE_STA][0];
        uint8_t line = ((line_bcd >> 4) * 10) + (line_bcd & 0xF);

        FuriString* sta_key = furi_string_alloc_printf("%04X", sta_id);
        FuriString* sta_name = furi_string_alloc();
        Storage* storage = furi_record_open(RECORD_STORAGE);
        FlipperFormat* ff = flipper_format_file_alloc(storage);

        if(flipper_format_file_open_existing(ff, EXT_PATH("nfc/assets/shmrt_id.nfc"))) {
            flipper_format_read_string(ff, furi_string_get_cstr(sta_key), sta_name);
        }
        flipper_format_free(ff);
        furi_record_close(RECORD_STORAGE);

        if(furi_string_size(sta_name) > 0) {
            furi_string_cat_printf(parsed_data, "STA: %s\n", furi_string_get_cstr(sta_name));
            furi_string_cat_printf(parsed_data, "Line %d\n", line);
        } else {
            furi_string_cat_printf(parsed_data, "STA ID: %04X\n", sta_id);
        }

        furi_string_free(sta_key);
        furi_string_free(sta_name);

        uint8_t machine = data->pages[SHMRT_PAGE_STA][2];
        furi_string_cat_printf(parsed_data, "Machine ID: %X/%X\n", sta_id, machine);

        uint16_t cashier = (uint16_t)(data->pages[SHMRT_PAGE_FARE][2] << 8) |
                           data->pages[SHMRT_PAGE_FARE][3];
        if(cashier == 0) {
            furi_string_cat_printf(parsed_data, "Issued via: TVM\n");
        } else {
            furi_string_cat_printf(parsed_data, "Issued via: DESK\n");
            furi_string_cat_printf(parsed_data, "Cashier ID: %X\n", cashier);
        }

        parsed = true;
    } while(false);

    return parsed;
}

static const NfcSupportedCardsPlugin shmrt_plugin = {
    .protocol = NfcProtocolFudanFm11rf005,
    .verify = shmrt_verify,
    .read = NULL,
    .parse = shmrt_parse,
};

static const FlipperAppPluginDescriptor shmrt_plugin_descriptor = {
    .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
    .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
    .entry_point = &shmrt_plugin,
};

const FlipperAppPluginDescriptor* shmrt_plugin_ep(void) {
    return &shmrt_plugin_descriptor;
}
