#include "fudan_fm11rf005_poller_sync.h"
#include "fudan_fm11rf005_poller_i.h"

#include <furi.h>

#define TAG "FudanPollerSync"
#define FUDAN_POLLER_FLAG_CMD_COMPLETE (1UL << 0)

typedef struct {
    FuriThreadId thread_id;
    FudanFm11rf005Error error;
} FudanFm11rf005PollerSyncCtx;

static NfcCommand fudan_fm11rf005_poller_sync_callback(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);

    FudanFm11rf005PollerSyncCtx* poller_ctx = context;
    FudanFm11rf005PollerEvent* fudan_event = event.event_data;

    if(fudan_event->type == FudanFm11rf005PollerEventTypeSuccess) {
        poller_ctx->error = FudanFm11rf005ErrorNone;
    } else if(fudan_event->type == FudanFm11rf005PollerEventTypeError) {
        poller_ctx->error = fudan_event->data->error;
    }

    furi_thread_flags_set(poller_ctx->thread_id, FUDAN_POLLER_FLAG_CMD_COMPLETE);
    return NfcCommandStop;
}

FudanFm11rf005Error
    fudan_fm11rf005_poller_sync_read_all(Nfc* nfc, FudanFm11rf005Data* data) {
    furi_assert(nfc);
    furi_assert(data);

    FudanFm11rf005PollerSyncCtx poller_ctx = {
        .thread_id = furi_thread_get_current_id(),
        .error = FudanFm11rf005ErrorNone,
    };

    NfcPoller* poller = nfc_poller_alloc(nfc, NfcProtocolFudanFm11rf005);
    nfc_poller_start(poller, fudan_fm11rf005_poller_sync_callback, &poller_ctx);

    uint32_t flags = furi_thread_flags_wait(
        FUDAN_POLLER_FLAG_CMD_COMPLETE, FuriFlagWaitAny, FuriWaitForever);
    if(flags & FUDAN_POLLER_FLAG_CMD_COMPLETE) {
        nfc_poller_stop(poller);
    }

    const FudanFm11rf005Data* poller_data = nfc_poller_get_data(poller);
    fudan_fm11rf005_copy(data, poller_data);

    nfc_poller_free(poller);
    return poller_ctx.error;
}
