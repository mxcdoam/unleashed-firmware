#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
//#include "../../api/elsy/elsytools.h"

typedef struct {
    uint64_t a;
    uint64_t b;
} MfClassicKeyPair;

typedef struct {
    uint32_t card_number;
    uint8_t valid_since_day;
    uint8_t valid_since_month;
    uint8_t valid_since_year;
    uint8_t valid_till_day;
    uint8_t valid_till_month;
    uint8_t valid_till_year;
    uint8_t metro_use_day;
    uint8_t metro_use_month;
    uint8_t metro_use_year;
    uint8_t metro_use_hour;
    uint8_t metro_use_minute;
    uint8_t valid_for_days;
} __attribute__((packed)) BepkData;

static const MfClassicKeyPair bepk_v1[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x2aba9519f574, .b = 0xcb9a1f2d7368},
    {.a = 0x84fd7f7a12b6, .b = 0xc7c0adb3284f},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7},
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb},
    {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0xffffffffffff, .b = 0x71f3a315ad26},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
};

static const MfClassicKeyPair bepk_v2[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x2aba9519f574, .b = 0xcb9a1f2d7368},
    {.a = 0x84fd7f7a12b6, .b = 0xc7c0adb3284f},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7},
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb},
    {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0x000000000000, .b = 0x71f3a315ad26},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xa0a1a2a3a4a5, .b = 0x103c08acceb2},
};

static const MfClassicKeyPair bepk_v3[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7},
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb},
    {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0x000000000000, .b = 0x71f3a315ad26},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xa0a1a2a3a4a5, .b = 0x103c08acceb2},
};

static const MfClassicKeyPair bepk_v4[] = {
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x2aba9519f574, .b = 0xcb9a1f2d7368},
    {.a = 0x84fd7f7a12b6, .b = 0xc7c0adb3284f},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7},
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb},
    {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0xffffffffffff, .b = 0x71f3a315ad26},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
};

static bool auth_and_check_data(Nfc* nfc) {
    MfClassicBlock block_data;
    MfClassicType type;

    if(mf_classic_poller_sync_detect_type(nfc, &type) != MfClassicErrorNone) return false;
    if(type != MfClassicType1k) return false;

    MfClassicKeyType key_type = MfClassicKeyTypeA;
    MfClassicKey mf_key;
    bit_lib_num_to_bytes_be(bepk_v1[8].a, sizeof(MfClassicKey), mf_key.data);
    if(mf_classic_poller_sync_auth(nfc, 8, &mf_key, key_type, NULL) != MfClassicErrorNone)
        return false;

    if(mf_classic_poller_sync_read_block(nfc, 34, &mf_key, key_type, &block_data) !=
       MfClassicErrorNone)
        return false;

    if(block_data.data[1] > 16) return false;
    if(block_data.data[2] < 1 || block_data.data[2] > 12) return false;
    if(block_data.data[3] < 1 || block_data.data[3] > 31) return false;
    if(block_data.data[4] > 23) return false;
    if(block_data.data[5] > 59) return false;

    bit_lib_num_to_bytes_be(bepk_v1[15].a, sizeof(MfClassicKey), mf_key.data);
    if(mf_classic_poller_sync_auth(nfc, 60, &mf_key, key_type, NULL) != MfClassicErrorNone) {
        // Try v2/v3 sec15 key for cards with sec15=0xa0a1a2a3a4a5
        bit_lib_num_to_bytes_be(bepk_v2[15].a, sizeof(MfClassicKey), mf_key.data);
        if(mf_classic_poller_sync_auth(nfc, 60, &mf_key, key_type, NULL) != MfClassicErrorNone)
            return false;
    }

    return true;
}

static bool bepk_verify(Nfc* nfc) {
    return auth_and_check_data(nfc);
}

static bool bepk_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type;
        if(mf_classic_poller_sync_detect_type(nfc, &type) != MfClassicErrorNone) break;
        if(type != MfClassicType1k) break;

        data->type = type;

        MfClassicKey key = {0};
        MfClassicAuthContext auth_ctx = {};
        const MfClassicKeyPair* keys;

        bit_lib_num_to_bytes_be(bepk_v1[2].a, sizeof(MfClassicKey), key.data);
        bool sec2_auth_ok =
            mf_classic_poller_sync_auth(nfc, 8, &key, MfClassicKeyTypeA, &auth_ctx);
        bit_lib_num_to_bytes_be(bepk_v1[0].a, sizeof(MfClassicKey), key.data);
        bool sec0_auth_ok =
            mf_classic_poller_sync_auth(nfc, 0, &key, MfClassicKeyTypeA, &auth_ctx);
        if(!sec2_auth_ok) {
            keys = bepk_v3;
        } else if(!sec0_auth_ok) {
            keys = bepk_v4;
        } else {
            bool sec15_auth_ok =
                mf_classic_poller_sync_auth(nfc, 60, &key, MfClassicKeyTypeA, &auth_ctx) ==
                MfClassicErrorNone;
            keys = sec15_auth_ok ? bepk_v2 : bepk_v1;
        }
        MfClassicDeviceKeys device_keys = {.key_a_mask = 0, .key_b_mask = 0};

        for(uint8_t i = 0; i < mf_classic_get_total_sectors_num(MfClassicType1k); i++) {
            bit_lib_num_to_bytes_be(keys[i].a, sizeof(MfClassicKey), device_keys.key_a[i].data);
            FURI_BIT_SET(device_keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(keys[i].b, sizeof(MfClassicKey), device_keys.key_b[i].data);
            FURI_BIT_SET(device_keys.key_b_mask, i);
        }

        MfClassicError error = mf_classic_poller_sync_read(nfc, &device_keys, data);
        if(error == MfClassicErrorNotPresent) break;

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone) ||
                  (error == MfClassicErrorPartialRead && mf_classic_is_sector_read(data, 8));
    } while(false);
    mf_classic_free(data);
    return is_read;
}

static bool bepk_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;
    BepkData ticket_data = {0};

    if(data->type != MfClassicType1k) return false;

    const MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 8);
    const uint64_t key_a =
        bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
    if(key_a != bepk_v1[8].a) return false;

    // Verify BEPK identity: sec0 = 0xa0a1a2a3a4a5 (v1-v3) or sec2 = 0x2aba9519f574 (v1,v2,v4)
    const MfClassicSectorTrailer* sec_tr0 = mf_classic_get_sector_trailer_by_sector(data, 0);
    const uint64_t key_a_0 =
        bit_lib_bytes_to_num_be(sec_tr0->key_a.data, COUNT_OF(sec_tr0->key_a.data));
    const MfClassicSectorTrailer* sec_tr2 = mf_classic_get_sector_trailer_by_sector(data, 2);
    const uint64_t key_a_2 =
        bit_lib_bytes_to_num_be(sec_tr2->key_a.data, COUNT_OF(sec_tr2->key_a.data));
    if(key_a_0 != 0xa0a1a2a3a4a5 && key_a_2 != 0x2aba9519f574) return false;

    // Reject student cards: sector 15 key A differs between card types
    const MfClassicSectorTrailer* sec_tr15 = mf_classic_get_sector_trailer_by_sector(data, 15);
    const uint64_t key_a_15 =
        bit_lib_bytes_to_num_be(sec_tr15->key_a.data, COUNT_OF(sec_tr15->key_a.data));
    if(key_a_15 == 0xa0a1a2a3a4a5 || key_a_15 == 0x3b21c684392e) return false;

    if(data->block[34].data[2] < 1 || data->block[34].data[2] > 12) return false;
    if(data->block[34].data[3] < 1 || data->block[34].data[3] > 31) return false;
    if(data->block[34].data[4] > 23) return false;
    if(data->block[34].data[5] > 59) return false;

    for(uint8_t i = 0; i < 4; i++) {
        ticket_data.card_number = ticket_data.card_number << 8 | data->block[0].data[3 - i];
    }
    uint32_t num_first_part =
        ticket_data.card_number / 10000000; // Gets the leading digits (e.g., 9xxx)
    uint32_t num_second_part = ticket_data.card_number % 10000000; // Gets the trailing 7 digits

    ticket_data.valid_since_day = data->block[40].data[4];
    ticket_data.valid_since_month = data->block[40].data[3];
    ticket_data.valid_since_year = data->block[40].data[2];
    ticket_data.valid_till_day = data->block[40].data[7];
    ticket_data.valid_till_month = data->block[40].data[6];
    ticket_data.valid_till_year = data->block[40].data[5];
    ticket_data.metro_use_day = data->block[34].data[3];
    ticket_data.metro_use_month = data->block[34].data[2];
    ticket_data.metro_use_year = data->block[34].data[1];
    ticket_data.metro_use_hour = data->block[34].data[4];
    ticket_data.metro_use_minute = data->block[34].data[5];

    ticket_data.valid_for_days = data->block[36].data[0];

    furi_string_printf(
        parsed_data, "\e#BEPK Card SPb\nNumber:9%03ld.%07ld\n", num_first_part, num_second_part);
    if(ticket_data.valid_since_day <= 31 && ticket_data.valid_since_month <= 12 &&
       ticket_data.valid_since_year <= 16)
        furi_string_cat_printf(
            parsed_data,
            " \nValid from:%02d-%02d-%4d",
            ticket_data.valid_since_day,
            ticket_data.valid_since_month,
            2000 + ticket_data.valid_since_year);
    if(ticket_data.valid_till_day <= 31 && ticket_data.valid_till_month <= 12 &&
       ticket_data.valid_till_year <= 16)
        furi_string_cat_printf(
            parsed_data,
            "\nValid till:%02d-%02d-%4d",
            ticket_data.valid_till_day,
            ticket_data.valid_since_month,
            2000 + ticket_data.valid_till_year);

    if(ticket_data.metro_use_day <= 31 && ticket_data.metro_use_month <= 12 &&
       ticket_data.metro_use_year <= 16 && ticket_data.metro_use_hour <= 23 &&
       ticket_data.metro_use_minute <= 59)
        furi_string_cat_printf(
            parsed_data,
            "\nLast use :%02d-%02d-%4d %02d:%02d\n",
            ticket_data.metro_use_day,
            ticket_data.metro_use_month,
            2000 + ticket_data.metro_use_year,
            ticket_data.metro_use_hour,
            ticket_data.metro_use_minute);

    if(ticket_data.valid_for_days > 0)
        furi_string_cat_printf(parsed_data, "\nValid for %d days\n", ticket_data.valid_for_days);
    parsed = true;
    return parsed;
}

static const NfcSupportedCardsPlugin bepk_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = bepk_verify,
    .read = bepk_read,
    .parse = bepk_parse,
};

__attribute__((used)) const FlipperAppPluginDescriptor* bepk_plugin_ep() {
    static const FlipperAppPluginDescriptor plugin_descriptor = {
        .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
        .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
        .entry_point = &bepk_plugin,
    };
    return &plugin_descriptor;
}
