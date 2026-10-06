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

} __attribute__((packed)) PensionData;

static const MfClassicKeyPair pension_keyset_v0[] = {
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x000000000000, .b = 0x7b5312d208c9},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x000000000000, .b = 0x2b780f1ed75b},
    {.a = 0x2066f4727129, .b = 0xf7a65799c6ee},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0x000000000000, .b = 0x3546315121c0},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x000000000000, .b = 0x71f3a315ad26},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x51044efb5aab, .b = 0xebdc720dd1ce},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
};

static const MfClassicKeyPair pension_keyset_v1[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0x000000000000, .b = 0x7ddf9e3f4020},
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
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

static const MfClassicKeyPair pension_keyset_v2[] = {
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
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x000000000000, .b = 0x71f3a315ad26},
    {.a = 0xac70ca327a04, .b = 0xf29411c2663c},
    {.a = 0x51044efb5aab, .b = 0xebdc720dd1ce},
    {.a = 0xa0a1a2a3a4a5, .b = 0x103c08acceb2},
};

static const MfClassicKeyPair pension_keyset_v3[] = {
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

static const MfClassicKeyPair pension_keyset_v4[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5}, {.a = 0x000000000000, .b = 0x7ddf9e3f4020},
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5}, {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0x000000000000, .b = 0x2b780f1ed75b}, {.a = 0x2066f4727129, .b = 0xf7a65799c6ee},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7}, {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0x000000000000, .b = 0x71f3a315ad26}, {.a = 0xac70ca327a04, .b = 0xf29411c2663c},
    {.a = 0x51044efb5aab, .b = 0xebdc720dd1ce}, {.a = 0xa0a1a2a3a4a5, .b = 0x103c08acceb2},
    {.a = 0xa8e1f1b93c16, .b = 0xae627138074e}, {.a = 0x2d43467f4cf8, .b = 0x011e07b93eeb},
    {.a = 0x4423ba2c467d, .b = 0x127548b166e5}, {.a = 0xbcc41dc309ef, .b = 0xe61816608327},
    {.a = 0xb25027e96660, .b = 0x8cf510835e79}, {.a = 0xd894b9722bdb, .b = 0x5a541812d292},
    {.a = 0x83647c87a453, .b = 0xcd93265a31ac}, {.a = 0x3f9933c49776, .b = 0xf55d814c07a1},
    {.a = 0xc0b0945918d3, .b = 0xb0251edf369b}, {.a = 0x55da2189f21d, .b = 0xabb9f02a8bd5},
    {.a = 0xe77daf4744e3, .b = 0xf1bb4b3e0fea}, {.a = 0xa90366d4b34b, .b = 0xf3c0934e0b17},
    {.a = 0x3a73f3d39527, .b = 0x79a4494b80f6}, {.a = 0xae79cb9397fe, .b = 0xfcf754346837},
    {.a = 0x4ea11a7e0c5e, .b = 0x408eeed7d9de}, {.a = 0xee67bf66e383, .b = 0xe61c90b39477},
    {.a = 0x33f5712fda91, .b = 0xc9a523812845}, {.a = 0xb9f358260e9b, .b = 0x83ea94e11343},
    {.a = 0x5baacb3a8072, .b = 0x61784dd70731}, {.a = 0x0f56e3486850, .b = 0xa2f23b6a4c7a},
    {.a = 0xc88a2cdafc0d, .b = 0x5cd9ea88417b}, {.a = 0x4e0ac43b8e96, .b = 0xc8fd1304064c},
    {.a = 0x25b74b2bcdd3, .b = 0x86ba36382806}, {.a = 0xf597af12d6db, .b = 0x0a14750ec85a},
};

static const MfClassicKeyPair pension_keyset_v5[] = {
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5}, {.a = 0xa0a1a2a3a4a5, .b = 0xb0b1b2b3b4b5},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0xffffffffffff, .b = 0xffffffffffff},
    {.a = 0xffffffffffff, .b = 0xffffffffffff}, {.a = 0x2066f4727129, .b = 0xf7a65799c6ee},
    {.a = 0x26973ea74321, .b = 0xd27058c6e2c7}, {.a = 0xeb0a8ff88ade, .b = 0x578a9ada41e3},
    {.a = 0xea0fd73cb149, .b = 0x29c35fa068fb}, {.a = 0xc76bf71a2509, .b = 0x9ba241db3f56},
    {.a = 0x000000000000, .b = 0x71f3a315ad26}, {.a = 0xac70ca327a04, .b = 0xf29411c2663c},
    {.a = 0x51044efb5aab, .b = 0xebdc720dd1ce}, {.a = 0xa0a1a2a3a4a5, .b = 0x103c08acceb2},
    {.a = 0xa8e1f1b93c16, .b = 0xae627138074e}, {.a = 0x2d43467f4cf8, .b = 0x011e07b93eeb},
    {.a = 0x4423ba2c467d, .b = 0x127548b166e5}, {.a = 0xbcc41dc309ef, .b = 0xe61816608327},
    {.a = 0xb25027e96660, .b = 0x8cf510835e79}, {.a = 0xd894b9722bdb, .b = 0x5a541812d292},
    {.a = 0x83647c87a453, .b = 0xcd93265a31ac}, {.a = 0x3f9933c49776, .b = 0xf55d814c07a1},
    {.a = 0xc0b0945918d3, .b = 0xb0251edf369b}, {.a = 0x55da2189f21d, .b = 0xabb9f02a8bd5},
    {.a = 0xe77daf4744e3, .b = 0xf1bb4b3e0fea}, {.a = 0xa90366d4b34b, .b = 0xf3c0934e0b17},
    {.a = 0x3a73f3d39527, .b = 0x79a4494b80f6}, {.a = 0xae79cb9397fe, .b = 0xfcf754346837},
    {.a = 0x4ea11a7e0c5e, .b = 0x408eeed7d9de}, {.a = 0xee67bf66e383, .b = 0xe61c90b39477},
    {.a = 0x33f5712fda91, .b = 0xc9a523812845}, {.a = 0xb9f358260e9b, .b = 0x83ea94e11343},
    {.a = 0x5baacb3a8072, .b = 0x61784dd70731}, {.a = 0x0f56e3486850, .b = 0xa2f23b6a4c7a},
    {.a = 0xc88a2cdafc0d, .b = 0x5cd9ea88417b}, {.a = 0x4e0ac43b8e96, .b = 0xc8fd1304064c},
    {.a = 0x25b74b2bcdd3, .b = 0x86ba36382806}, {.a = 0xf597af12d6db, .b = 0x0a14750ec85a},
};

static bool pension_verify(Nfc* nfc) {
    MfClassicBlock block_data;
    MfClassicType type;

    if(mf_classic_poller_sync_detect_type(nfc, &type) != MfClassicErrorNone) return false;

    MfClassicKeyType key_type = MfClassicKeyTypeA;
    MfClassicKey mf_key;
    const uint8_t sec15_block = mf_classic_get_first_block_num_of_sector(15);

    bit_lib_num_to_bytes_be(pension_keyset_v1[15].a, sizeof(MfClassicKey), mf_key.data);
    if(mf_classic_poller_sync_auth(nfc, sec15_block, &mf_key, key_type, NULL) !=
       MfClassicErrorNone) {
        bit_lib_num_to_bytes_be(pension_keyset_v0[15].a, sizeof(MfClassicKey), mf_key.data);
        if(mf_classic_poller_sync_auth(nfc, sec15_block, &mf_key, key_type, NULL) !=
           MfClassicErrorNone)
            return false;
    }

    if(mf_classic_poller_sync_read_block(nfc, sec15_block + 1, &mf_key, key_type, &block_data) !=
       MfClassicErrorNone)
        return false;

    // Pension markers: 0x24 0x22, 0x24 0x19, 0x28 0x13
    if(block_data.data[0] == 0x24 && block_data.data[1] == 0x22) return true;
    if(block_data.data[0] == 0x24 && block_data.data[1] == 0x19) return true;
    if(block_data.data[0] == 0x28 && block_data.data[1] == 0x13) return true;

    return false;
}

static bool pension_read(Nfc* nfc, NfcDevice* device) {
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

        const MfClassicKeyPair* keysets_1k[] = {
            pension_keyset_v1, pension_keyset_v2, pension_keyset_v3, pension_keyset_v0};
        const MfClassicKeyPair* keysets_4k[] = {pension_keyset_v4, pension_keyset_v5};
        size_t num_keysets = (type == MfClassicType1k) ? 4 : 2;
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

static bool pension_parse(const NfcDevice* device, FuriString* parsed_data) {
    furi_assert(device);
    const MfClassicData* data = nfc_device_get_data(device, NfcProtocolMfClassic);
    bool parsed = false;
    PensionData ticket_data = {0};

    if(data->type != MfClassicType1k && data->type != MfClassicType4k) return false;

    // Check sector 8 key A matches our keysets
    const MfClassicSectorTrailer* sec_tr = mf_classic_get_sector_trailer_by_sector(data, 8);
    const uint64_t key_a =
        bit_lib_bytes_to_num_be(sec_tr->key_a.data, COUNT_OF(sec_tr->key_a.data));
    bool sec8_pension_v0 = (key_a == pension_keyset_v0[8].a);
    bool sec8_free = (key_a == pension_keyset_v1[8].a);
    if(!sec8_pension_v0 && !sec8_free) return false;

    if(sec8_free) {
        const MfClassicSectorTrailer* sec_tr15 = mf_classic_get_sector_trailer_by_sector(data, 15);
        const uint64_t key_a_15 =
            bit_lib_bytes_to_num_be(sec_tr15->key_a.data, COUNT_OF(sec_tr15->key_a.data));
        if(key_a_15 == 0xffffffffffff) return false;
    }

    // Reject free cards: sector 12 key A == 0xacffffffffff is unique to free
    const MfClassicSectorTrailer* sec_tr12 = mf_classic_get_sector_trailer_by_sector(data, 12);
    const uint64_t key_a_12 =
        bit_lib_bytes_to_num_be(sec_tr12->key_a.data, COUNT_OF(sec_tr12->key_a.data));
    if(key_a_12 == 0xacffffffffff) return false;

    // Pension markers: 0x24 0x22, 0x24 0x19, 0x28 0x13
    uint8_t b61_0 = data->block[61].data[0];
    uint8_t b61_1 = data->block[61].data[1];

    bool is_pension = false;
    if(b61_0 == 0x24 && b61_1 == 0x22) is_pension = true;
    if(b61_0 == 0x24 && b61_1 == 0x19) is_pension = true;
    if(b61_0 == 0x28 && b61_1 == 0x13) is_pension = true;

    if(!is_pension) return false;

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

    // block[52]: surname CP1251
    {
        size_t i = 1;
        size_t len = 0;
        while(i < sizeof(data->block[52].data) && data->block[52].data[i] != 0x20) {
            ticket_data.surname[len++] = data->block[52].data[i++];
        }
        ticket_data.surname[len] = 0;
    }

    // blocks[56-57]: name + patronymic CP1251
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

    const char* pension_type;
    if(b61_0 == 0x24 && b61_1 == 0x22) {
        pension_type = "Pension";
    } else if(b61_0 == 0x24 && b61_1 == 0x19) {
        pension_type = "Pension";
    } else {
        pension_type = "Pension";
    }

    furi_string_printf(
        parsed_data,
        "\e#SPb Concession Card\nType: %s\nNo: %s\nValid since: %02d-%02d-%4d\nValid until: %02d-%02d-%4d\nLast metro use :%02d-%02d-%4d %02d:%02d\nLast TAT use:%02d-%02d-%4d %02d:%02d\n",
        pension_type,
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
        "\n******OWNER INFO******\nLastname: %s\nName: %s\nPatronym: %s\n",
        furi_string_get_cstr(surname_out),
        furi_string_get_cstr(name_out),
        furi_string_get_cstr(patronymic_out));

    furi_string_cat_printf(
        parsed_data,
        "\n*****DOC INFO*****\nType: %s\n",
        ticket_data.id_type == ELSY_DOC_PASSPORT         ? "PASSPORT" :
        ticket_data.id_type == ELSY_DOC_FOREIGN_PASSPORT ? "FOREIGN PASSPORT" :
                                                           "BIRTH CERTIFICATE");

    if(ticket_data.id_type == ELSY_DOC_PASSPORT) {
        furi_string_cat_printf(
            parsed_data,
            "Region: %s\nSeries: %s\nNumber: %06lu\n",
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

static const NfcSupportedCardsPlugin pension_plugin = {
    .protocol = NfcProtocolMfClassic,
    .verify = pension_verify,
    .read = pension_read,
    .parse = pension_parse,
};

__attribute__((used)) const FlipperAppPluginDescriptor* cncs_pension_plugin_ep() {
    static const FlipperAppPluginDescriptor plugin_descriptor = {
        .appid = NFC_SUPPORTED_CARD_PLUGIN_APP_ID,
        .ep_api_version = NFC_SUPPORTED_CARD_PLUGIN_API_VERSION,
        .entry_point = &pension_plugin,
    };
    return &plugin_descriptor;
}
