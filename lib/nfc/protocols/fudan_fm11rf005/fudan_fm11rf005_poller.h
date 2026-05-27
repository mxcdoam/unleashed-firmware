#pragma once

#include "fudan_fm11rf005.h"
#include <lib/nfc/nfc.h>
#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct FudanFm11rf005Poller FudanFm11rf005Poller;

typedef enum {
    FudanFm11rf005PollerEventTypeError,
    FudanFm11rf005PollerEventTypeReady,
    FudanFm11rf005PollerEventTypeSuccess,
} FudanFm11rf005PollerEventType;

typedef union {
    FudanFm11rf005Error error;
} FudanFm11rf005PollerEventData;

typedef struct {
    FudanFm11rf005PollerEventType type;
    FudanFm11rf005PollerEventData* data;
} FudanFm11rf005PollerEvent;

#ifdef __cplusplus
}
#endif
