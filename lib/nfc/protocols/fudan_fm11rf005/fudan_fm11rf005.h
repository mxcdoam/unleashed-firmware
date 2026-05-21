#pragma once

#include <nfc/protocols/nfc_device_base_i.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FUDAN_FM11RF005_UID_SIZE (4U)
#define FUDAN_FM11RF005_PAGE_SIZE (4U)
#define FUDAN_FM11RF005_PAGE_NUM (16U)
#define FUDAN_FM11RF005_CARD_SIZE (FUDAN_FM11RF005_PAGE_NUM * FUDAN_FM11RF005_PAGE_SIZE)
#define FUDAN_FM11RF005_CID_SIZE (2U)
#define FUDAN_FM11RF005_MID_SIZE (2U)
#define FUDAN_FM11RF005_KEY_SIZE (6U)
#define FUDAN_FM11RF005_READABLE_PAGE_NUM (8U)
#define FUDAN_FM11RF005_DATA_SIZE (FUDAN_FM11RF005_PAGE_NUM * FUDAN_FM11RF005_PAGE_SIZE)

#define FUDAN_FM11RF005_CMD_READ (0x30U)
#define FUDAN_FM11RF005_CMD_WRITE (0xA0U)

#define FUDAN_FM11RF005_FDT_POLL_FC (6000U)
#define FUDAN_FM11RF005_GUARD_TIME_US (5000U)
#define FUDAN_FM11RF005_POLL_POLL_MIN_US (1280U)

typedef enum {
    FudanFm11rf005ErrorNone,
    FudanFm11rf005ErrorNotPresent,
    FudanFm11rf005ErrorCommunication,
    FudanFm11rf005ErrorTimeout,
} FudanFm11rf005Error;

typedef enum {
    FudanFm11rf005TypeUnknown,
    FudanFm11rf005TypeFM11RF005,
    FudanFm11rf005TypeFM11RF005SH,

    FudanFm11rf005TypeNum,
} FudanFm11rf005Type;

typedef struct {
    uint8_t uid[FUDAN_FM11RF005_UID_SIZE];
    uint8_t pages[FUDAN_FM11RF005_PAGE_NUM][FUDAN_FM11RF005_PAGE_SIZE];
    FudanFm11rf005Type type;
    uint16_t atqa;
    uint8_t sak;
} FudanFm11rf005Data;

extern const NfcDeviceBase nfc_device_fudan_fm11rf005;

FudanFm11rf005Data* fudan_fm11rf005_alloc(void);

void fudan_fm11rf005_free(FudanFm11rf005Data* data);

void fudan_fm11rf005_reset(FudanFm11rf005Data* data);

void fudan_fm11rf005_copy(FudanFm11rf005Data* dest, const FudanFm11rf005Data* src);

bool fudan_fm11rf005_verify(const FudanFm11rf005Data* data, const FuriString* device_type);

bool fudan_fm11rf005_load(FudanFm11rf005Data* data, FlipperFormat* ff, uint32_t version);

bool fudan_fm11rf005_save(const FudanFm11rf005Data* data, FlipperFormat* ff);

bool fudan_fm11rf005_is_equal(const FudanFm11rf005Data* data, const FudanFm11rf005Data* other);

const char*
    fudan_fm11rf005_get_name(const FudanFm11rf005Data* data, NfcDeviceNameType name_type);

const uint8_t* fudan_fm11rf005_get_uid(const FudanFm11rf005Data* data, size_t* uid_len);

bool fudan_fm11rf005_set_uid(FudanFm11rf005Data* data, const uint8_t* uid, size_t uid_len);

FudanFm11rf005Data* fudan_fm11rf005_get_base_data(const FudanFm11rf005Data* data);

FudanFm11rf005Type fudan_fm11rf005_get_type_from_atqa_sak(uint16_t atqa);

#ifdef __cplusplus
}
#endif
