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

} __attribute__((packed)) FreeData;

static const MfClassicKeyPair free_keyset_v1[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xe56ac127dd45, .b = 0x19fc84a3784b}, {.a = 0x77dabc9825e1, .b = 0x9764fec3154a},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0x2066f4727129, .b = 0xf7a65799c6ee},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7}, {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0xacffffffffff, .b = 0x71f3a315ad26}, {.a = 0xac70ca327a04, .b = 0xf29411c2663c},
    {.a = 0x51044efb5aab, .b = 0xebdc720dd1ce}, {.a = 0xa0a1a2a3a4a5, .b = 0x103c08acceb2},
    {.a = 0x72f96bdd3714, .b = 0x462225cd34cf}, {.a = 0x044ce1872bc3, .b = 0x8c90c70cff4a},
    {.a = 0xbc2d1791dec1, .b = 0xca96a487de0b}, {.a = 0x8791b2ccb5c4, .b = 0xc956c3b80da3},
    {.a = 0x8e26e45e7d65, .b = 0x8e65b3af7d22}, {.a = 0x0f318130ed18, .b = 0x0c420a20e056},
    {.a = 0x045ceca15535, .b = 0x31bec3d9e510}, {.a = 0x9d993c5d4ef4, .b = 0x86120e488abf},
    {.a = 0xc65d4eaa645b, .b = 0xb69d40d1a439}, {.a = 0x3a8a139c20b4, .b = 0x8818a9c5d406},
    {.a = 0xbaff3053b496, .b = 0x4b7cb25354d3}, {.a = 0x7413b599c4ea, .b = 0xb0a2aaf3a1ba},
    {.a = 0x0ce7cd2cc72b, .b = 0xfa1fbb3f0f1f}, {.a = 0x0be5fac8b06a, .b = 0x6f95887a4fd3},
    {.a = 0x0eb23cc8110b, .b = 0x04dc35277635}, {.a = 0xbc4580b7f20b, .b = 0xd0a4131fb290},
    {.a = 0x7a396f0d633d, .b = 0xad2bdc097023}, {.a = 0xa3faa6daff67, .b = 0x7600e889adf9},
    {.a = 0xfd8705e721b0, .b = 0x296fc317a513}, {.a = 0x22052b480d11, .b = 0xe19504c39461},
    {.a = 0xa7141147d430, .b = 0xff16014fefc7}, {.a = 0x8a8d88151a00, .b = 0x038b5f9b5a2a},
    {.a = 0xb27addfb64b0, .b = 0x152fd0c420a7}, {.a = 0x7259fa0197c6, .b = 0x5583698df085},
};

static const MfClassicKeyPair free_keyset_v2[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5}, {.a = 0x000000000000, .b = 0x7ddf9e3f4020},
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5}, {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xe56ac127dd45, .b = 0x19fc84a3784b}, {.a = 0x77dabc9825e1, .b = 0x9764fec3154a},
    {.a = 0x000000000000, .b = 0x2b780f1ed75b}, {.a = 0x2066f4727129, .b = 0xf7a65799c6ee},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7}, {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0xacffffffffff, .b = 0x71f3a315ad26}, {.a = 0xac70ca327a04, .b = 0xf29411c2663c},
    {.a = 0x51044efb5aab, .b = 0xebdc720dd1ce}, {.a = 0xa0a1a2a3a4a5, .b = 0x103c08acceb2},
    {.a = 0x72f96bdd3714, .b = 0x462225cd34cf}, {.a = 0x044ce1872bc3, .b = 0x8c90c70cff4a},
    {.a = 0xbc2d1791dec1, .b = 0xca96a487de0b}, {.a = 0x8791b2ccb5c4, .b = 0xc956c3b80da3},
    {.a = 0x8e26e45e7d65, .b = 0x8e65b3af7d22}, {.a = 0x0f318130ed18, .b = 0x0c420a20e056},
    {.a = 0x045ceca15535, .b = 0x31bec3d9e510}, {.a = 0x9d993c5d4ef4, .b = 0x86120e488abf},
    {.a = 0xc65d4eaa645b, .b = 0xb69d40d1a439}, {.a = 0x46d78e850a7e, .b = 0xa470f8130991},
    {.a = 0x42e9b54e51ab, .b = 0x0231b86df52e}, {.a = 0x0f01ceff2742, .b = 0x6fec74559ca7},
    {.a = 0xb81f2b0c2f66, .b = 0xa7e2d95f0003}, {.a = 0x9ea3387a63c1, .b = 0x437e59f57561},
    {.a = 0x0eb23cc8110b, .b = 0x04dc35277635}, {.a = 0xbc4580b7f20b, .b = 0xd0a4131fb290},
    {.a = 0x7a396f0d633d, .b = 0xad2bdc097023}, {.a = 0xa3faa6daff67, .b = 0x7600e889adf9},
    {.a = 0xfd8705e721b0, .b = 0x296fc317a513}, {.a = 0x22052b480d11, .b = 0xe19504c39461},
    {.a = 0xa7141147d430, .b = 0xff16014fefc7}, {.a = 0x8a8d88151a00, .b = 0x038b5f9b5a2a},
    {.a = 0xb27addfb64b0, .b = 0x152fd0c420a7}, {.a = 0x7259fa0197c6, .b = 0x5583698df085},
};

static const MfClassicKeyPair free_keyset_v3[] = {
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xe56ac127dd45, .b = 0x19fc84a3784b}, {.a = 0x77dabc9825e1, .b = 0x9764fec3154a},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x72f96bdd3714, .b = 0x462225cd34cf}, {.a = 0x044ce1872bc3, .b = 0x8c90c70cff4a},
    {.a = 0xbc2d1791dec1, .b = 0xca96a487de0b}, {.a = 0x8791b2ccb5c4, .b = 0xc956c3b80da3},
    {.a = 0x8e26e45e7d65, .b = 0x8e65b3af7d22}, {.a = 0x0f318130ed18, .b = 0x0c420a20e056},
    {.a = 0x045ceca15535, .b = 0x31bec3d9e510}, {.a = 0x9d993c5d4ef4, .b = 0x86120e488abf},
    {.a = 0xc65d4eaa645b, .b = 0xb69d40d1a439}, {.a = 0x46d78e850a7e, .b = 0xa470f8130991},
    {.a = 0x42e9b54e51ab, .b = 0x0231b86df52e}, {.a = 0x0f01ceff2742, .b = 0x6fec74559ca7},
    {.a = 0xb81f2b0c2f66, .b = 0xa7e2d95f0003}, {.a = 0x9ea3387a63c1, .b = 0x437e59f57561},
    {.a = 0x0eb23cc8110b, .b = 0x04dc35277635}, {.a = 0xbc4580b7f20b, .b = 0xd0a4131fb290},
    {.a = 0x7a396f0d633d, .b = 0xad2bdc097023}, {.a = 0xa3faa6daff67, .b = 0x7600e889adf9},
    {.a = 0xfd8705e721b0, .b = 0x296fc317a513}, {.a = 0x22052b480d11, .b = 0xe19504c39461},
    {.a = 0xa7141147d430, .b = 0xff16014fefc7}, {.a = 0x8a8d88151a00, .b = 0x038b5f9b5a2a},
    {.a = 0xb27addfb64b0, .b = 0x152fd0c420a7}, {.a = 0x7259fa0197c6, .b = 0x5583698df085},
};

static bool free_verify(Nfc* nfc) {
    MfClassicBlock block_data;
    MfClassicType type;

    if(mf_classic_poller_sync_detect_type(nfc, &type) != MfClassicErrorNone) return false;

    MfClassicKeyType key_type = MfClassicKeyTypeA;
    MfClassicKey mf_key;
    const uint8_t sec15_block = mf_classic_get_first_block_num_of_sector(15);

    // Try sector 15 auth with free v1 key, fall back to v3 for 4K cards
    bit_lib_num_to_bytes_be(free_keyset_v1[15].a, sizeof(MfClassicKey), mf_key.data);
    if(mf_classic_poller_sync_auth(nfc, sec15_block, &mf_key, key_type, NULL) !=
       MfClassicErrorNone) {
        bit_lib_num_to_bytes_be(free_keyset_v3[15].a, sizeof(MfClassicKey), mf_key.data);
        if(mf_classic_poller_sync_auth(nfc, sec15_block, &mf_key, key_type, NULL) !=
           MfClassicErrorNone)
            return false;
    }

    if(mf_classic_poller_sync_read_block(nfc, sec15_block + 1, &mf_key, key_type, &block_data) !=
       MfClassicErrorNone)
        return false;

    // Free concession sig: 0x30 0x91 or 0x28 0x13
    if(block_data.data[0] == 0x30 && block_data.data[1] == 0x91) return true;
    if(block_data.data[0] == 0x28 && block_data.data[1] == 0x13) {
        // 0x28 0x13 is shared with pension — disambiguate via sec12 key (0xacffffffffff = free only)
        const uint8_t sec12_block = mf_classic_get_first_block_num_of_sector(12);
        bit_lib_num_to_bytes_be(free_keyset_v1[12].a, sizeof(MfClassicKey), mf_key.data);
        if(mf_classic_poller_sync_auth(nfc, sec12_block, &mf_key, key_type, NULL) !=
           MfClassicErrorNone)
            return false;
        return true;
    }

    return false;
}

static bool free_read(Nfc* nfc, NfcDevice* device) {
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

        const MfClassicKeyPair* keysets_1k[] = {free_keyset_v1, free_keyset_v2};
        const MfClassicKeyPair* keysets_4k[] = {free_keyset_v1, free_keyset_v2, free_keyset_v3};
        size_t num_keysets = (type == MfClassicType1k) ? 2 : 3;
        const MfClassicKeyPair** keysets = (type == MfClassicType1k) ? keysets_1k : keysets_4k;

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

static bool free_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;
    FreeData ticket_data = {0};

    if(data->type != MfClassicType1k && data->type != MfClassicType4k) return false;

    // Sector 12 key A must be 0xacffffffffff (unique to free cards)
    const MfClassicSectorTrailer* sec_tr12 = mf_classic_get_sector_trailer_by_sector(data, 12);
    const uint64_t key_a_12 =
        bit_lib_bytes_to_num_be(sec_tr12->key_a.data, COUNT_OF(sec_tr12->key_a.data));
    if(key_a_12 != free_keyset_v1[12].a) return false;

    // Free concession sig: 0x30 0x91 or 0x28 0x13
    uint8_t b61_0 = data->block[61].data[0];
    uint8_t b61_1 = data->block[61].data[1];

    bool is_free = false;
    if(b61_0 == 0x30 && b61_1 == 0x91) is_free = true;
    if(b61_0 == 0x28 && b61_1 == 0x13) is_free = true;

    if(!is_free) return false;

    // block[32]: validity dates
    ticket_data.valid_since_year = data->block[32].data[7];
    ticket_data.valid_since_month = data->block[32].data[8];
    ticket_data.valid_since_day = data->block[32].data[9];
    ticket_data.valid_till_year = data->block[32].data[10];
    ticket_data.valid_till_month = data->block[32].data[11];
    ticket_data.valid_till_day = data->block[32].data[12];

    // block[33]: doc-series + number
    ticket_data.id_info[0] = data->block[33].data[3];
    ticket_data.id_info[1] = data->block[33].data[4];
    ticket_data.id_info[2] = data->block[33].data[5];
    ticket_data.id_info[3] = data->block[33].data[6];
    ticket_data.id_info[4] = data->block[33].data[7];
    ticket_data.id_info[5] = data->block[33].data[9];
    ticket_data.id_info[6] = data->block[33].data[10];
    ticket_data.id_info[7] = data->block[33].data[11];

    // block[34]: last metro use
    ticket_data.metro_use_year = data->block[34].data[1];
    ticket_data.metro_use_month = data->block[34].data[2];
    ticket_data.metro_use_day = data->block[34].data[3];
    ticket_data.metro_use_hour = data->block[34].data[4];
    ticket_data.metro_use_minute = data->block[34].data[5];

    // block[36]: valid for N days
    ticket_data.valid_for_days = data->block[36].data[0];

    // block[48]: last TAT use
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
        "\e#SPb Concession Card\nType: Free Travel\nNo: %s\nValid since: %02d-%02d-%4d\nValid until: %02d-%02d-%4d\nLast metro use :%02d-%02d-%4d %02d:%02d\nLast TAT use:%02d-%02d-%4d %02d:%02d\n",
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
        "\n*****OWNER INFO******\nLName: %s\nFName: %s\nPatrnm: %s\n",
        furi_string_get_cstr(surname_out),
        furi_string_get_cstr(name_out),
        furi_string_get_cstr(patronymic_out));

    furi_string_cat_printf(
        parsed_data,
        "\n*****DOC INFO*****\nType: %s\n",
        ticket_data.id_type == ELSY_DOC_PASSPORT         ? "RU PASSPORT" :
        ticket_data.id_type == ELSY_DOC_FOREIGN_PASSPORT ? "FRGN PASSPORT" :
                                                           "BIRTH CERT");

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

static const NfcSupportedCardsPlugin free_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = free_verify,
    .read = free_read,
    .parse = free_parse,
};

__attribute__((used)) const FlipperAppPluginDescriptor* cncs_free_plugin_ep() {
    static const FlipperAppPluginDescriptor plugin_descriptor = {
        .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
        .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
        .entry_point = &free_plugin,
    };
    return &plugin_descriptor;
}
