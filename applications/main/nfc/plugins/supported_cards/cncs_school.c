#include "nfc_supported_card_plugin.h"
#include <flipper_application/flipper_application.h>
#include <nfc/nfc_device.h>
#include <bit_lib/bit_lib.h>
#include <nfc/protocols/mf_classic/mf_classic_poller_sync.h>
#include "../../api/elsy/elsytools.h"

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
    uint8_t tat_use_day;
    uint8_t tat_use_month;
    uint8_t tat_use_year;
    uint8_t tat_use_hour;
    uint8_t tat_use_minute;
    uint8_t valid_for_days;
    uint8_t id_info[8];
    uint8_t surname[32];
    uint8_t name[32];
    uint8_t patronymic[32];
    uint8_t id_type;
    FuriString* firstname;
    FuriString* patronym;
    FuriString* lastname;
    FuriString* doc_region;
    FuriString* doc_series;
    FuriString* doc_number;

} __attribute__((packed)) SchoolData;

static const MfClassicKeyPair school_keyset[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0x000000000000, .b = 0x7ddf9e3f4020},
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0x000000000000, .b = 0x4bb911b0abf3},
    {.a = 0x000000000000, .b = 0x8f5ddc4d20d7},
    {.a = 0x000000000000, .b = 0x2b780f1ed75b},
    {.a = 0x2066f4727129, .b = 0xf7a65799c6ee},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7},
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0x000000000000, .b = 0x3546315121c0},
    {.a = 0x000000000000, .b = 0x4bf04d117ed5},
    {.a = 0x000000000000, .b = 0x71f3a315ad26},
    {.a = 0xac70ca327a04, .b = 0xf29411c2663c},
    {.a = 0x51044efb5aab, .b = 0xebdc720dd1ce},
    {.a = 0xa0a1a2a3a4a5, .b = 0x103c08acceb2},
};

static const MfClassicKeyPair school_keyset_v4[] = {
    {.a = 0x21e781f34a5c, .b = 0xa0582fbe71e9},
    {.a = 0x3d206ca92814, .b = 0x245534783842},
    {.a = 0x2ea63e94b464, .b = 0x36307f716f1a},
    {.a = 0xdf8ef4313010, .b = 0x2bd90c39091e},
    {.a = 0xf778fe111847, .b = 0xbd397851a4f5},
    {.a = 0xf778fe111847, .b = 0xbd397851a4f5},
    {.a = 0xca8df40aad54, .b = 0x48f4b2a984cc},
    {.a = 0x2066f4727129, .b = 0xf7a65799c6ee},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7},
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xe16eb927f7dc, .b = 0xb196185ab4ba},
    {.a = 0x246f85bce39d, .b = 0xf26390c55303},
    {.a = 0x000000000000, .b = 0x71f3a315ad26},
    {.a = 0xac70ca327a04, .b = 0xf29411c2663c},
    {.a = 0x51044efb5aab, .b = 0xebdc720dd1ce},
    {.a = 0x3b21c684392e, .b = 0xc619826a5a5b},
};

static bool school_verify(Nfc* nfc) {
    MfClassicBlock block_data;
    MfClassicType type;

    if(mf_classic_poller_sync_detect_type(nfc, &type) != MfClassicErrorNone) return false;

    MfClassicKeyType key_type = MfClassicKeyTypeA;
    MfClassicKey mf_key;
    const uint8_t sec15_block = mf_classic_get_first_block_num_of_sector(15);

    bit_lib_num_to_bytes_be(school_keyset[15].a, sizeof(MfClassicKey), mf_key.data);
    if(mf_classic_poller_sync_auth(nfc, sec15_block, &mf_key, key_type, NULL) !=
       MfClassicErrorNone) {
        bit_lib_num_to_bytes_be(school_keyset_v4[15].a, sizeof(MfClassicKey), mf_key.data);
        if(mf_classic_poller_sync_auth(nfc, sec15_block, &mf_key, key_type, NULL) !=
           MfClassicErrorNone)
            return false;
    }

    if(mf_classic_poller_sync_read_block(nfc, sec15_block, &mf_key, key_type, &block_data) !=
       MfClassicErrorNone)
        return false;
    uint8_t b60_type = block_data.data[0];

    if(mf_classic_poller_sync_read_block(nfc, sec15_block + 1, &mf_key, key_type, &block_data) !=
       MfClassicErrorNone)
        return false;

    if(b60_type == 0x01) {
        if(block_data.data[0] != 0x26 || block_data.data[1] != 0x06) return false;
    } else {
        return false;
    }

    return true;
}

static bool school_read(Nfc* nfc, NfcDevice* device) {
    furi_assert(nfc);
    furi_assert(device);

    bool is_read = false;

    MfClassicData* data = mf_classic_alloc();
    nfc_device_copy_data(device, NfcProtocolMfClassic, data);

    do {
        MfClassicType type;
        if(mf_classic_poller_sync_detect_type(nfc, &type) != MfClassicErrorNone) break;

        data->type = type;

        MfClassicKey key = {0};
        MfClassicAuthContext auth_ctx = {};

        const MfClassicKeyPair* keysets[] = {school_keyset, school_keyset_v4};
        size_t num_keysets = 2;

        const MfClassicKeyPair* keys = NULL;

        for(size_t k = 0; k < num_keysets; k++) {
            bit_lib_num_to_bytes_be(keysets[k][0].a, sizeof(MfClassicKey), key.data);
            if(mf_classic_poller_sync_auth(nfc, 0, &key, MfClassicKeyTypeA, &auth_ctx) ==
               MfClassicErrorNone) {
                keys = keysets[k];
                break;
            }
        }

        if(!keys) break;

        uint8_t total_sectors = mf_classic_get_total_sectors_num(type);
        MfClassicDeviceKeys device_keys = {.key_a_mask = 0, .key_b_mask = 0};
        for(uint8_t i = 0; i < total_sectors; i++) {
            bit_lib_num_to_bytes_be(keys[i].a, sizeof(MfClassicKey), device_keys.key_a[i].data);
            FURI_BIT_SET(device_keys.key_a_mask, i);
            bit_lib_num_to_bytes_be(keys[i].b, sizeof(MfClassicKey), device_keys.key_b[i].data);
            FURI_BIT_SET(device_keys.key_b_mask, i);
        }

        MfClassicError error = mf_classic_poller_sync_read(nfc, &device_keys, data);
        if(error == MfClassicErrorNotPresent) break;

        nfc_device_set_data(device, NfcProtocolMfClassic, data);

        is_read = (error == MfClassicErrorNone) || (mf_classic_is_sector_read(data, 8));
    } while(false);

    mf_classic_free(data);
    return is_read;
}

static bool school_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;
    SchoolData ticket_data = {0};

    if(data->type != MfClassicType1k && data->type != MfClassicType4k) return false;

    const MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 8);
    const uint64_t key_a =
        bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
    if(key_a != school_keyset[8].a) return false;

    const MfClassicSectorTrailer* sec_tr15 = mf_classic_get_sector_trailer_by_sector(data, 15);
    const uint64_t key_a_15 =
        bit_lib_bytes_to_num_be(sec_tr15->key_a.data, COUNT_OF(sec_tr15->key_a.data));
    if(key_a_15 == 0xffffffffffff) return false;

    if(data->block[61].data[0] != 0x26 || data->block[61].data[1] != 0x06) return false;
    ticket_data.valid_since_year = data->block[32].data[7];
    ticket_data.valid_since_month = data->block[32].data[8];
    ticket_data.valid_since_day = data->block[32].data[9];
    ticket_data.valid_till_year = data->block[32].data[10];
    ticket_data.valid_till_month = data->block[32].data[11];
    ticket_data.valid_till_day = data->block[32].data[12];
    ticket_data.id_info[0] = data->block[33].data[3];
    ticket_data.id_info[1] = data->block[33].data[4];
    ticket_data.id_info[2] = data->block[33].data[5];
    ticket_data.id_info[3] = data->block[33].data[6];
    ticket_data.id_info[4] = data->block[33].data[7];
    ticket_data.id_info[5] = data->block[33].data[9];
    ticket_data.id_info[6] = data->block[33].data[10];
    ticket_data.id_info[7] = data->block[33].data[11];
    ticket_data.metro_use_year = data->block[34].data[1];
    ticket_data.metro_use_month = data->block[34].data[2];
    ticket_data.metro_use_day = data->block[34].data[3];
    ticket_data.metro_use_hour = data->block[34].data[4];
    ticket_data.metro_use_minute = data->block[34].data[5];
    ticket_data.valid_for_days = data->block[36].data[0];
    ticket_data.tat_use_year = data->block[48].data[9];
    ticket_data.tat_use_month = data->block[48].data[10];
    ticket_data.tat_use_day = data->block[48].data[11];
    ticket_data.tat_use_hour = data->block[48].data[12];
    ticket_data.tat_use_minute = data->block[48].data[13];

    {
        size_t i = 1;
        size_t len = 0;
        while(i < sizeof(data->block[52].data) && data->block[52].data[i] != 0x20) {
            ticket_data.surname[len++] = data->block[52].data[i++];
        }
        ticket_data.surname[len] = 0;
    }

    {
        const uint8_t* raw = data->block[56].data;
        size_t i = 1;
        size_t len = 0;

        while(i < sizeof(data->block[56].data) && raw[i] != 0x20) {
            ticket_data.name[len++] = raw[i++];
        }
        ticket_data.name[len] = 0;

        while(i < sizeof(data->block[56].data) && raw[i] == 0x20)
            i++;

        len = 0;
        while(i < sizeof(data->block[56].data) && raw[i] != 0x20) {
            ticket_data.patronymic[len++] = raw[i++];
        }
        if(i >= sizeof(data->block[56].data) && len > 0) {
            raw = data->block[57].data;
            i = 0;
            while(i < sizeof(data->block[57].data) && raw[i] != 0x20) {
                ticket_data.patronymic[len++] = raw[i++];
            }
        }
        ticket_data.patronymic[len] = 0;
    }

    FuriString* region = furi_string_alloc();
    FuriString* id_series = furi_string_alloc();
    FuriString* surname_out = furi_string_alloc();
    FuriString* name_out = furi_string_alloc();
    FuriString* patronymic_out = furi_string_alloc();

    get_elsy_doc(
        ticket_data.id_info,
        ticket_data.surname,
        ticket_data.name,
        ticket_data.patronymic,
        &ticket_data.id_type,
        region,
        id_series,
        NULL,
        surname_out,
        name_out,
        patronymic_out);

    uint32_t id_number = (uint32_t)ticket_data.id_info[5] |
                         ((uint32_t)ticket_data.id_info[6] << 8) |
                         ((uint32_t)ticket_data.id_info[7] << 16);

    // Card number from UID
    uint64_t card_number = 0;
    size_t uid_len = 0;
    const uint8_t* uid = mf_classic_get_uid(data, &uid_len);
    const uint8_t* temp_ptr = &uid[0];
    uint8_t card_number_tmp[uid_len];

    if(uid_len == 4) {
        for(size_t i = 0; i < 4; i++) {
            card_number_tmp[i] = temp_ptr[3 - i];
        }
    } else if(uid_len == 7) {
        for(size_t i = 0; i < 7; i++) {
            card_number_tmp[i] = temp_ptr[6 - i];
        }
    }

    for(size_t i = 0; i < uid_len; i++) {
        card_number = (card_number << 8) | card_number_tmp[i];
    }

    FuriString* card_number_s = furi_string_alloc();
    furi_string_cat_printf(card_number_s, "%lld", card_number);
    FuriString* card_str = furi_string_alloc_set_str("9643 3078 ");
    for(uint8_t i = 0; i < 24; i += 4) {
        for(uint8_t j = 0; j < 4; j++) {
            furi_string_push_back(card_str, furi_string_get_char(card_number_s, i + j));
        }
        furi_string_push_back(card_str, ' ');
    }
    furi_string_free(card_number_s);

    furi_string_printf(
        parsed_data,
        "\e#SPb Concession Card\nType: School Pass\nNo: %s\nValid since: %02d-%02d-%4d\nValid until: %02d-%02d-%4d\nLast metro use :%02d-%02d-%4d %02d:%02d\nLast TAT use:%02d-%02d-%4d %02d:%02d\n",
        furi_string_get_cstr(card_str),
        ticket_data.valid_since_day,
        ticket_data.valid_since_month,
        ticket_data.valid_since_year + 2000,
        ticket_data.valid_till_day,
        ticket_data.valid_till_month,
        ticket_data.valid_till_year + 2000,
        ticket_data.metro_use_day,
        ticket_data.metro_use_month,
        ticket_data.metro_use_year + 2000,
        ticket_data.metro_use_hour,
        ticket_data.metro_use_minute,
        ticket_data.tat_use_day,
        ticket_data.tat_use_month,
        ticket_data.tat_use_year + 2000,
        ticket_data.tat_use_hour,
        ticket_data.tat_use_minute);

    furi_string_cat_printf(
        parsed_data,
        "*****OWNER INFO*****\nLName: %s\nFName: %s\nPatrnm: %s\n",
        furi_string_get_cstr(surname_out),
        furi_string_get_cstr(name_out),
        furi_string_get_cstr(patronymic_out));

    furi_string_cat_printf(
        parsed_data,
        "******DOC INFO*****\nType: %s\n",
        ticket_data.id_type == ELSY_DOC_PASSPORT         ? "PASSPORT" :
        ticket_data.id_type == ELSY_DOC_FOREIGN_PASSPORT ? "FOREIGN PASSPORT" :
                                                           "BIRTH CERTIFICATE");

    if(ticket_data.id_type == ELSY_DOC_PASSPORT) {
        furi_string_cat_printf(
            parsed_data,
            "Reg.: %s\nSeries: %s\nNumber: %06lu\n",
            furi_string_get_cstr(region),
            furi_string_get_cstr(id_series),
            (unsigned long)id_number);
    } else {
        furi_string_cat_printf(
            parsed_data,
            "Series: %s\nNumber: %06lu\n",
            furi_string_get_cstr(id_series),
            (unsigned long)id_number);
    }

    furi_string_free(card_str);
    furi_string_free(region);
    furi_string_free(id_series);
    furi_string_free(surname_out);
    furi_string_free(name_out);
    furi_string_free(patronymic_out);

    parsed = true;
    return parsed;
}

static const NfcSupportedCardsPlugin school_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = school_verify,
    .read = school_read,
    .parse = school_parse,
};

__attribute__((used)) const FlipperAppPluginDescriptor* cncs_school_plugin_ep() {
    static const FlipperAppPluginDescriptor plugin_descriptor = {
        .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
        .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
        .entry_point = &school_plugin,
    };
    return &plugin_descriptor;
}
