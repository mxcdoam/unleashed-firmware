#include "fudan_fm11rf005.h"

#include <furi.h>
#include <flipper_format.h>
#include <nfc/nfc_common.h>

#define FUDAN_PROTOCOL_NAME "Fudan"
#define FUDAN_TYPE_KEY      "Type"
#define FUDAN_ATQA_KEY      "ATQA"
#define FUDAN_SAK_KEY       "SAK"

const NfcDeviceBase nfc_device_fudan_fm11rf005 = {
    .protocol_name = FUDAN_PROTOCOL_NAME,
    .alloc = (NfcDeviceAlloc)fudan_fm11rf005_alloc,
    .free = (NfcDeviceFree)fudan_fm11rf005_free,
    .reset = (NfcDeviceReset)fudan_fm11rf005_reset,
    .copy = (NfcDeviceCopy)fudan_fm11rf005_copy,
    .verify = NULL,
    .load = (NfcDeviceLoad)fudan_fm11rf005_load,
    .save = (NfcDeviceSave)fudan_fm11rf005_save,
    .is_equal = (NfcDeviceEqual)fudan_fm11rf005_is_equal,
    .get_name = (NfcDeviceGetName)fudan_fm11rf005_get_name,
    .get_uid = (NfcDeviceGetUid)fudan_fm11rf005_get_uid,
    .set_uid = (NfcDeviceSetUid)fudan_fm11rf005_set_uid,
    .get_base_data = (NfcDeviceGetBaseData)fudan_fm11rf005_get_base_data,
};

FudanFm11rf005Data* fudan_fm11rf005_alloc(void) {
    FudanFm11rf005Data* data = malloc(sizeof(FudanFm11rf005Data));
    return data;
}

void fudan_fm11rf005_free(FudanFm11rf005Data* data) {
    furi_check(data);
    free(data);
}

void fudan_fm11rf005_reset(FudanFm11rf005Data* data) {
    furi_check(data);
    memset(data, 0, sizeof(FudanFm11rf005Data));
}

void fudan_fm11rf005_copy(FudanFm11rf005Data* dest, const FudanFm11rf005Data* src) {
    furi_check(dest);
    furi_check(src);
    *dest = *src;
}

FudanFm11rf005Type fudan_fm11rf005_get_type_from_atqa_sak(uint16_t atqa, uint8_t sak) {
    if((sak & 0x0A) == 0x0A) {
        if((atqa & 0x0003) == 0x0003) return FudanFm11rf005TypeFM11RF005SH;
        if((atqa & 0x0005) == 0x0005) return FudanFm11rf005TypeFM11RF005;
    }
    if((sak & 0x53) == 0x53) return FudanFm11rf005TypeFM11RF005;
    return FudanFm11rf005TypeUnknown;
}

bool fudan_fm11rf005_load(FudanFm11rf005Data* data, FlipperFormat* ff, uint32_t version) {
    furi_check(data);
    furi_check(ff);

    bool parsed = false;
    do {
        if(version < NFC_UNIFIED_FORMAT_VERSION) break;

        if(!flipper_format_read_hex(ff, "UID", data->uid, FUDAN_FM11RF005_UID_SIZE)) break;
        if(!flipper_format_read_hex(ff, "Pages", (uint8_t*)data->pages, FUDAN_FM11RF005_DATA_SIZE))
            break;
        if(!flipper_format_read_hex(
               ff, FUDAN_ATQA_KEY, (uint8_t*)&data->atqa, FUDAN_FM11RF005_CID_SIZE))
            break;
        if(!flipper_format_read_hex(ff, FUDAN_SAK_KEY, &data->sak, 1)) break;

        data->type = fudan_fm11rf005_get_type_from_atqa_sak(data->atqa, data->sak);

        parsed = true;
    } while(false);

    return parsed;
}

bool fudan_fm11rf005_save(const FudanFm11rf005Data* data, FlipperFormat* ff) {
    furi_check(data);
    furi_check(ff);

    bool saved = false;

    do {
        if(!flipper_format_write_comment_cstr(ff, FUDAN_PROTOCOL_NAME " specific data")) break;
        if(!flipper_format_write_hex(ff, "UID", data->uid, FUDAN_FM11RF005_UID_SIZE)) break;
        if(!flipper_format_write_hex(
               ff, "Pages", (const uint8_t*)data->pages, FUDAN_FM11RF005_DATA_SIZE))
            break;
        if(!flipper_format_write_hex(
               ff, FUDAN_ATQA_KEY, (const uint8_t*)&data->atqa, FUDAN_FM11RF005_CID_SIZE))
            break;
        if(!flipper_format_write_hex(ff, FUDAN_SAK_KEY, &data->sak, 1)) break;

        saved = true;
    } while(false);

    return saved;
}

bool fudan_fm11rf005_is_equal(const FudanFm11rf005Data* data, const FudanFm11rf005Data* other) {
    furi_check(data);
    furi_check(other);

    return memcmp(data, other, sizeof(FudanFm11rf005Data)) == 0;
}

const char* fudan_fm11rf005_get_name(const FudanFm11rf005Data* data, NfcDeviceNameType name_type) {
    furi_check(data);

    if(name_type == NfcDeviceNameTypeFull) {
        if(data->type == FudanFm11rf005TypeFM11RF005) {
            return "Fudan 005";
        } else if(data->type == FudanFm11rf005TypeFM11RF005SH) {
            return "Fudan 005SH";
        }
        return "Fudan ???";
    }

    return FUDAN_PROTOCOL_NAME;
}

const uint8_t* fudan_fm11rf005_get_uid(const FudanFm11rf005Data* data, size_t* uid_len) {
    furi_check(data);

    if(uid_len) {
        *uid_len = FUDAN_FM11RF005_UID_SIZE;
    }
    return data->uid;
}

bool fudan_fm11rf005_set_uid(FudanFm11rf005Data* data, const uint8_t* uid, size_t uid_len) {
    furi_check(data);
    furi_check(uid);

    const bool valid = (uid_len == FUDAN_FM11RF005_UID_SIZE);
    if(valid) {
        memcpy(data->uid, uid, uid_len);
    }
    return valid;
}

FudanFm11rf005Data* fudan_fm11rf005_get_base_data(const FudanFm11rf005Data* data) {
    UNUSED(data);
    return NULL;
}
