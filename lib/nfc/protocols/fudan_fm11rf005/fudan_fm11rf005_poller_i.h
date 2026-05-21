#pragma once

#include "fudan_fm11rf005_poller.h"
#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FUDAN_FM11RF005_POLLER_MAX_BUFFER_SIZE (256U)

typedef enum {
    FudanFm11rf005PollerStateIdle,
    FudanFm11rf005PollerStateActivate,
    FudanFm11rf005PollerStateReadPages,

    FudanFm11rf005PollerStateNum,
} FudanFm11rf005PollerState;

struct FudanFm11rf005Poller {
    Nfc* nfc;
    FudanFm11rf005PollerState state;
    FudanFm11rf005Data* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    NfcGenericEvent general_event;
    FudanFm11rf005PollerEvent fudan_event;
    FudanFm11rf005PollerEventData fudan_event_data;
    NfcGenericCallback callback;
    void* context;

    uint8_t current_page;
};

const FudanFm11rf005Data* fudan_fm11rf005_poller_get_data(FudanFm11rf005Poller* instance);

FudanFm11rf005Error fudan_fm11rf005_poller_frame_exchange(
    FudanFm11rf005Poller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

FudanFm11rf005Error
    fudan_fm11rf005_poller_sdd_exchange(FudanFm11rf005Poller* instance, BitBuffer* rx_buffer);

#ifdef __cplusplus
}
#endif
