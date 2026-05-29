#include "fudan_fm11rf005_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>
#include <nfc/helpers/iso14443_crc.h>

#include <furi.h>

const FudanFm11rf005Data* fudan_fm11rf005_poller_get_data(FudanFm11rf005Poller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    return instance->data;
}

static FudanFm11rf005Poller* fudan_fm11rf005_poller_alloc(Nfc* nfc) {
    furi_assert(nfc);

    FudanFm11rf005Poller* instance = malloc(sizeof(FudanFm11rf005Poller));
    instance->nfc = nfc;
    instance->tx_buffer = bit_buffer_alloc(FUDAN_FM11RF005_POLLER_MAX_BUFFER_SIZE);
    instance->rx_buffer = bit_buffer_alloc(FUDAN_FM11RF005_POLLER_MAX_BUFFER_SIZE);

    nfc_config(instance->nfc, NfcModePoller, NfcTechIso14443a);
    nfc_set_guard_time_us(instance->nfc, FUDAN_FM11RF005_GUARD_TIME_US);
    nfc_set_fdt_poll_fc(instance->nfc, FUDAN_FM11RF005_FDT_POLL_FC);
    nfc_set_fdt_poll_poll_us(instance->nfc, FUDAN_FM11RF005_POLL_POLL_MIN_US);

    instance->data = fudan_fm11rf005_alloc();

    instance->fudan_event.data = &instance->fudan_event_data;
    instance->general_event.protocol = NfcProtocolFudanFm11rf005;
    instance->general_event.event_data = &instance->fudan_event;
    instance->general_event.instance = instance;

    return instance;
}

static void fudan_fm11rf005_poller_free(FudanFm11rf005Poller* instance) {
    furi_assert(instance);
    furi_assert(instance->tx_buffer);
    furi_assert(instance->rx_buffer);
    furi_assert(instance->data);

    bit_buffer_free(instance->tx_buffer);
    bit_buffer_free(instance->rx_buffer);
    fudan_fm11rf005_free(instance->data);
    free(instance);
}

static void fudan_fm11rf005_poller_set_callback(
    FudanFm11rf005Poller* instance,
    NfcGenericCallback callback,
    void* context) {
    furi_assert(instance);
    furi_assert(callback);

    instance->callback = callback;
    instance->context = context;
}

static FudanFm11rf005Error fudan_fm11rf005_poller_process_error(NfcError error) {
    switch(error) {
    case NfcErrorNone:
        return FudanFm11rf005ErrorNone;
    case NfcErrorTimeout:
        return FudanFm11rf005ErrorTimeout;
    default:
        return FudanFm11rf005ErrorNotPresent;
    }
}

static FudanFm11rf005Error fudan_fm11rf005_poller_read_page(
    FudanFm11rf005Poller* instance,
    uint8_t page_num) {
    furi_assert(instance);
    furi_assert(page_num < FUDAN_FM11RF005_PAGE_NUM);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    uint8_t cmd[2] = {FUDAN_FM11RF005_CMD_READ, page_num};
    bit_buffer_copy_bytes(instance->tx_buffer, cmd, sizeof(cmd));
    iso14443_crc_append(Iso14443CrcTypeA, instance->tx_buffer);

    NfcError error = nfc_poller_trx(
        instance->nfc, instance->tx_buffer, instance->rx_buffer, FUDAN_FM11RF005_FDT_POLL_FC);
    if(error != NfcErrorNone) return fudan_fm11rf005_poller_process_error(error);

    if(!iso14443_crc_check(Iso14443CrcTypeA, instance->rx_buffer)) {
        return FudanFm11rf005ErrorCommunication;
    }

    iso14443_crc_trim(instance->rx_buffer);
    return FudanFm11rf005ErrorNone;
}

static FudanFm11rf005Error fudan_fm11rf005_poller_activate(
    FudanFm11rf005Poller* instance,
    FudanFm11rf005Data* data) {
    furi_assert(instance);
    furi_assert(data);

    fudan_fm11rf005_reset(data);

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    NfcError error = nfc_iso14443a_poller_trx_short_frame(
        instance->nfc,
        NfcIso14443aShortFrameSensReq,
        instance->rx_buffer,
        FUDAN_FM11RF005_FDT_POLL_FC);
    if(error != NfcErrorNone) return fudan_fm11rf005_poller_process_error(error);
    if(bit_buffer_get_size_bytes(instance->rx_buffer) < 2) return FudanFm11rf005ErrorCommunication;

    uint8_t cid[2];
    bit_buffer_write_bytes(instance->rx_buffer, cid, sizeof(cid));
    data->atqa = ((uint16_t)cid[1] << 8) | cid[0];

    bit_buffer_reset(instance->tx_buffer);
    bit_buffer_reset(instance->rx_buffer);

    uint8_t sel_cmd[1] = {0x93};
    bit_buffer_copy_bytes(instance->tx_buffer, sel_cmd, sizeof(sel_cmd));
    iso14443_crc_append(Iso14443CrcTypeA, instance->tx_buffer);

    NfcError nfc_err = nfc_iso14443a_poller_trx_custom_parity(
        instance->nfc, instance->tx_buffer, instance->rx_buffer, FUDAN_FM11RF005_FDT_POLL_FC);

    data->sak = 0;
    if(nfc_err == NfcErrorNone &&
       bit_buffer_get_size_bytes(instance->rx_buffer) >= 1) {
        data->sak = bit_buffer_get_byte(instance->rx_buffer, 0);
    }

    for(uint8_t i = 0; i < FUDAN_FM11RF005_PAGE_NUM; i++) {
        FudanFm11rf005Error page_err = fudan_fm11rf005_poller_read_page(instance, i);
        if(page_err == FudanFm11rf005ErrorNone &&
           bit_buffer_get_size_bytes(instance->rx_buffer) >= FUDAN_FM11RF005_PAGE_SIZE) {
            bit_buffer_write_bytes(instance->rx_buffer, data->pages[i], FUDAN_FM11RF005_PAGE_SIZE);
        } else {
            memset(data->pages[i], 0, FUDAN_FM11RF005_PAGE_SIZE);
        }
    }

    memcpy(data->uid, data->pages[1], FUDAN_FM11RF005_UID_SIZE);

    data->type = fudan_fm11rf005_get_type_from_atqa_sak(data->atqa, data->sak);

    return FudanFm11rf005ErrorNone;
}

static NfcCommand fudan_fm11rf005_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    FudanFm11rf005Poller* instance = context;
    NfcEvent* nfc_event = event.event_data;

    if(nfc_event->type == NfcEventTypePollerReady) {
        FudanFm11rf005Error error = fudan_fm11rf005_poller_activate(instance, instance->data);
        if(error == FudanFm11rf005ErrorNone) {
            instance->fudan_event.type = FudanFm11rf005PollerEventTypeSuccess;
        } else {
            instance->fudan_event.type = FudanFm11rf005PollerEventTypeError;
            instance->fudan_event_data.error = error;
        }
        instance->callback(instance->general_event, instance->context);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static bool fudan_fm11rf005_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolInvalid);

    bool protocol_detected = false;
    FudanFm11rf005Poller* instance = context;
    NfcEvent* nfc_event = event.event_data;

    if(nfc_event->type == NfcEventTypePollerReady) {
        FudanFm11rf005Error error = fudan_fm11rf005_poller_activate(instance, instance->data);
        if(error == FudanFm11rf005ErrorNone) {
            uint16_t atqa = instance->data->atqa;
            protocol_detected = ((atqa & 0x0003) == 0x0003) || ((atqa & 0x0005) == 0x0005);
        }
    }

    return protocol_detected;
}

const NfcPollerBase fudan_fm11rf005_poller = {
    .alloc = (NfcPollerAlloc)fudan_fm11rf005_poller_alloc,
    .free = (NfcPollerFree)fudan_fm11rf005_poller_free,
    .set_callback = (NfcPollerSetCallback)fudan_fm11rf005_poller_set_callback,
    .run = (NfcPollerRun)fudan_fm11rf005_poller_run,
    .detect = (NfcPollerDetect)fudan_fm11rf005_poller_detect,
    .get_data = (NfcPollerGetData)fudan_fm11rf005_poller_get_data,
};
