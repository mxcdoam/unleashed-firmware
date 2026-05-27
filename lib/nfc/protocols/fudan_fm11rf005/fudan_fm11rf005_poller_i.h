#pragma once

#include "fudan_fm11rf005_poller.h"
#include <toolbox/bit_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif

#define FUDAN_FM11RF005_POLLER_MAX_BUFFER_SIZE (32U)

struct FudanFm11rf005Poller {
    Nfc* nfc;
    FudanFm11rf005Data* data;
    BitBuffer* tx_buffer;
    BitBuffer* rx_buffer;

    NfcGenericEvent general_event;
    FudanFm11rf005PollerEvent fudan_event;
    FudanFm11rf005PollerEventData fudan_event_data;
    NfcGenericCallback callback;
    void* context;
};

const FudanFm11rf005Data* fudan_fm11rf005_poller_get_data(FudanFm11rf005Poller* instance);

FudanFm11rf005Error fudan_fm11rf005_poller_frame_exchange(
    FudanFm11rf005Poller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt);

#ifdef __cplusplus
}
#endif
