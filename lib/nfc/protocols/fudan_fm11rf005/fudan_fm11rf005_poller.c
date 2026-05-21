#include "fudan_fm11rf005_poller_i.h"

#include <nfc/protocols/nfc_poller_base.h>
#include <nfc/helpers/iso14443_crc.h>

#include <furi.h>

#define TAG "FudanPoller"

typedef NfcCommand (*FudanFm11rf005PollerStateHandler)(FudanFm11rf005Poller* instance);

const FudanFm11rf005Data* fudan_fm11rf005_poller_get_data(FudanFm11rf005Poller* instance) {
    furi_assert(instance);
    furi_assert(instance->data);
    return instance->data;
}

static FudanFm11rf005Poller* fudan_fm11rf005_poller_alloc(Nfc* nfc) {
    furi_assert(nfc);

    FudanFm11rf005Poller* instance = malloc(sizeof(FudanFm11rf005Poller));
    instance->nfc = nfc;
    instance->state = FudanFm11rf005PollerStateIdle;
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

FudanFm11rf005Error fudan_fm11rf005_poller_frame_exchange(
    FudanFm11rf005Poller* instance,
    const BitBuffer* tx_buffer,
    BitBuffer* rx_buffer,
    uint32_t fwt) {
    furi_assert(instance);
    furi_assert(tx_buffer);
    furi_assert(rx_buffer);

    const size_t tx_bytes = bit_buffer_get_size_bytes(tx_buffer);
    furi_assert(
        tx_bytes <= bit_buffer_get_capacity_bytes(instance->tx_buffer) - ISO14443_CRC_SIZE);

    bit_buffer_copy(instance->tx_buffer, tx_buffer);
    iso14443_crc_append(Iso14443CrcTypeA, instance->tx_buffer);

    FudanFm11rf005Error ret = FudanFm11rf005ErrorNone;

    do {
        NfcError error =
            nfc_poller_trx(instance->nfc, instance->tx_buffer, instance->rx_buffer, fwt);
        if(error != NfcErrorNone) {
            ret = fudan_fm11rf005_poller_process_error(error);
            break;
        }

        bit_buffer_copy(rx_buffer, instance->rx_buffer);
        if(!iso14443_crc_check(Iso14443CrcTypeA, instance->rx_buffer)) {
            ret = FudanFm11rf005ErrorCommunication;
            break;
        }

        iso14443_crc_trim(rx_buffer);
    } while(false);

    return ret;
}

FudanFm11rf005Error
    fudan_fm11rf005_poller_sdd_exchange(FudanFm11rf005Poller* instance, BitBuffer* rx_buffer) {
    furi_assert(instance);
    furi_assert(rx_buffer);

    FudanFm11rf005Error ret = FudanFm11rf005ErrorNone;

    do {
        NfcError error = nfc_iso14443a_poller_trx_sdd_frame(
            instance->nfc, instance->tx_buffer, instance->rx_buffer, FUDAN_FM11RF005_FDT_POLL_FC);
        if(error != NfcErrorNone) {
            ret = fudan_fm11rf005_poller_process_error(error);
            break;
        }

        bit_buffer_copy(rx_buffer, instance->rx_buffer);
    } while(false);

    return ret;
}

FudanFm11rf005Error
    fudan_fm11rf005_poller_activate(FudanFm11rf005Poller* instance, FudanFm11rf005Data* data) {
    furi_assert(instance);
    furi_assert(data);

    FudanFm11rf005Error ret = FudanFm11rf005ErrorNone;

    do {
        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        NfcError error = nfc_iso14443a_poller_trx_short_frame(
            instance->nfc,
            NfcIso14443aShortFrameSensReq,
            instance->rx_buffer,
            FUDAN_FM11RF005_FDT_POLL_FC);
        if(error != NfcErrorNone) {
            ret = fudan_fm11rf005_poller_process_error(error);
            break;
        }

        if(bit_buffer_get_size_bytes(instance->rx_buffer) < 2) {
            ret = FudanFm11rf005ErrorCommunication;
            break;
        }

        uint16_t atqa = 0;
        bit_buffer_write_bytes(instance->rx_buffer, &atqa, sizeof(atqa));
        data->atqa = atqa;
        FURI_LOG_D(TAG, "ATQA: 0x%04X", atqa);

        bit_buffer_reset(instance->tx_buffer);
        bit_buffer_reset(instance->rx_buffer);

        bit_buffer_set_size_bytes(instance->tx_buffer, 2);
        bit_buffer_set_byte(instance->tx_buffer, 0, 0x93);
        bit_buffer_set_byte(instance->tx_buffer, 1, 0x20);

        FudanFm11rf005Error sdd_err =
            fudan_fm11rf005_poller_sdd_exchange(instance, instance->rx_buffer);
        if(sdd_err != FudanFm11rf005ErrorNone) {
            FURI_LOG_W(TAG, "SDD failed, card activated with CID only");
            ret = FudanFm11rf005ErrorNone;
            break;
        }

        size_t rx_len = bit_buffer_get_size_bytes(instance->rx_buffer);
        FURI_LOG_D(TAG, "SDD response length: %d", rx_len);

        if(rx_len >= 5) {
            uint8_t uid_buf[4] = {0};
            bit_buffer_write_bytes(instance->rx_buffer, &uid_buf, 4);
            memcpy(data->uid, uid_buf, FUDAN_FM11RF005_UID_SIZE);

            bit_buffer_reset(instance->tx_buffer);
            bit_buffer_reset(instance->rx_buffer);

            uint8_t sel_cmd[7];
            sel_cmd[0] = 0x93;
            sel_cmd[1] = 0x70;
            memcpy(&sel_cmd[2], data->uid, 4);
            sel_cmd[6] = data->uid[0] ^ data->uid[1] ^ data->uid[2] ^ data->uid[3];
            bit_buffer_copy_bytes(instance->tx_buffer, sel_cmd, sizeof(sel_cmd));

            FudanFm11rf005Error sel_err = fudan_fm11rf005_poller_frame_exchange(
                instance, instance->tx_buffer, instance->rx_buffer, FUDAN_FM11RF005_FDT_POLL_FC);
            if(sel_err == FudanFm11rf005ErrorNone) {
                if(bit_buffer_get_size_bytes(instance->rx_buffer) >= 1) {
                    data->sak = bit_buffer_get_byte(instance->rx_buffer, 0);
                    data->type = fudan_fm11rf005_get_type_from_atqa_sak(data->atqa);
                }
            }
        }
    } while(false);

    return ret;
}

FudanFm11rf005Error fudan_fm11rf005_poller_read_page(
    FudanFm11rf005Poller* instance,
    uint8_t page,
    uint8_t* page_data) {
    furi_assert(instance);
    furi_assert(page < FUDAN_FM11RF005_PAGE_NUM);
    furi_assert(page_data);

    FudanFm11rf005Error ret = FudanFm11rf005ErrorNone;

    do {
        bit_buffer_reset(instance->tx_buffer);

        uint8_t cmd[2];
        cmd[0] = FUDAN_FM11RF005_CMD_READ;
        cmd[1] = page;
        bit_buffer_copy_bytes(instance->tx_buffer, cmd, sizeof(cmd));

        ret = fudan_fm11rf005_poller_frame_exchange(
            instance, instance->tx_buffer, instance->rx_buffer, FUDAN_FM11RF005_FDT_POLL_FC);
        if(ret != FudanFm11rf005ErrorNone) break;

        if(bit_buffer_get_size_bytes(instance->rx_buffer) < FUDAN_FM11RF005_PAGE_SIZE) {
            ret = FudanFm11rf005ErrorCommunication;
            break;
        }

        bit_buffer_write_bytes(instance->rx_buffer, page_data, FUDAN_FM11RF005_PAGE_SIZE);
    } while(false);

    return ret;
}

static NfcCommand fudan_fm11rf005_poller_state_idle_handler(FudanFm11rf005Poller* instance) {
    fudan_fm11rf005_reset(instance->data);
    instance->state = FudanFm11rf005PollerStateActivate;
    return NfcCommandContinue;
}

static NfcCommand fudan_fm11rf005_poller_state_activate_handler(FudanFm11rf005Poller* instance) {
    FudanFm11rf005Error error = fudan_fm11rf005_poller_activate(instance, instance->data);

    if(error == FudanFm11rf005ErrorNone) {
        instance->state = FudanFm11rf005PollerStateReadPages;
        instance->current_page = 0;
    } else {
        instance->fudan_event.type = FudanFm11rf005PollerEventTypeError;
        instance->fudan_event_data.error = error;
        instance->callback(instance->general_event, instance->context);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static NfcCommand fudan_fm11rf005_poller_state_read_handler(FudanFm11rf005Poller* instance) {
    FudanFm11rf005Error error = fudan_fm11rf005_poller_read_page(
        instance, instance->current_page, instance->data->pages[instance->current_page]);

    if(error != FudanFm11rf005ErrorNone) {
        memset(instance->data->pages[instance->current_page], 0, FUDAN_FM11RF005_PAGE_SIZE);
    }

    instance->current_page++;
    if(instance->current_page >= FUDAN_FM11RF005_PAGE_NUM) {
        instance->fudan_event.type = FudanFm11rf005PollerEventTypeSuccess;
        instance->callback(instance->general_event, instance->context);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static FudanFm11rf005PollerStateHandler
    fudan_fm11rf005_poller_state_handlers[FudanFm11rf005PollerStateNum] = {
        [FudanFm11rf005PollerStateIdle] = fudan_fm11rf005_poller_state_idle_handler,
        [FudanFm11rf005PollerStateActivate] = fudan_fm11rf005_poller_state_activate_handler,
        [FudanFm11rf005PollerStateReadPages] = fudan_fm11rf005_poller_state_read_handler,
};

static NfcCommand fudan_fm11rf005_poller_run(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.protocol == NfcProtocolInvalid);
    furi_assert(event.event_data);

    FudanFm11rf005Poller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    NfcCommand command = NfcCommandContinue;

    furi_assert(instance->state < FudanFm11rf005PollerStateNum);

    if(nfc_event->type == NfcEventTypePollerReady) {
        command = fudan_fm11rf005_poller_state_handlers[instance->state](instance);
    }

    return command;
}

static bool fudan_fm11rf005_poller_detect(NfcGenericEvent event, void* context) {
    furi_assert(context);
    furi_assert(event.event_data);
    furi_assert(event.instance);
    furi_assert(event.protocol == NfcProtocolInvalid);

    bool protocol_detected = false;
    FudanFm11rf005Poller* instance = context;
    NfcEvent* nfc_event = event.event_data;
    furi_assert(instance->state == FudanFm11rf005PollerStateIdle);

    if(nfc_event->type == NfcEventTypePollerReady) {
        FudanFm11rf005Error error = fudan_fm11rf005_poller_activate(instance, instance->data);
        protocol_detected = (error == FudanFm11rf005ErrorNone);
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
