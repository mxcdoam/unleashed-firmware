#include "fudan_fm11rf005.h"
#include "fudan_fm11rf005_render.h"

#include <nfc/protocols/fudan_fm11rf005/fudan_fm11rf005_poller.h>

#include "nfc/nfc_app_i.h"

#include "../nfc_protocol_support_common.h"
#include "../nfc_protocol_support_gui_common.h"

static void nfc_scene_info_on_enter_fudan(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const FudanFm11rf005Data* data = nfc_device_get_data(device, NfcProtocolFudanFm11rf005);

    FuriString* temp_str = furi_string_alloc();
    nfc_append_filename_string_when_present(instance, temp_str);
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_fudan_fm11rf005_info(data, NfcProtocolFormatTypeFull, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 64, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

static NfcCommand nfc_scene_read_poller_callback_fudan(NfcGenericEvent event, void* context) {
    furi_assert(event.protocol == NfcProtocolFudanFm11rf005);

    NfcApp* instance = context;
    const FudanFm11rf005PollerEvent* fudan_event = event.event_data;

    if(fudan_event->type == FudanFm11rf005PollerEventTypeSuccess) {
        nfc_device_set_data(
            instance->nfc_device,
            NfcProtocolFudanFm11rf005,
            nfc_poller_get_data(instance->poller));
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerSuccess);
        return NfcCommandStop;
    } else if(fudan_event->type == FudanFm11rf005PollerEventTypeError) {
        view_dispatcher_send_custom_event(instance->view_dispatcher, NfcCustomEventPollerFailure);
        return NfcCommandStop;
    }

    return NfcCommandContinue;
}

static void nfc_scene_read_on_enter_fudan(NfcApp* instance) {
    nfc_poller_start(instance->poller, nfc_scene_read_poller_callback_fudan, instance);
}

static void nfc_scene_read_success_on_enter_fudan(NfcApp* instance) {
    const NfcDevice* device = instance->nfc_device;
    const FudanFm11rf005Data* data = nfc_device_get_data(device, NfcProtocolFudanFm11rf005);

    FuriString* temp_str = furi_string_alloc();
    furi_string_cat_printf(
        temp_str, "\e#%s\n", nfc_device_get_name(device, NfcDeviceNameTypeFull));
    nfc_render_fudan_fm11rf005_info(data, NfcProtocolFormatTypeShort, temp_str);

    widget_add_text_scroll_element(
        instance->widget, 0, 0, 128, 52, furi_string_get_cstr(temp_str));

    furi_string_free(temp_str);
}

const NfcProtocolSupportBase nfc_protocol_support_fudan_fm11rf005 = {
    .features = NfcProtocolFeatureNone,

    .scene_info =
        {
            .on_enter = nfc_scene_info_on_enter_fudan,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read =
        {
            .on_enter = nfc_scene_read_on_enter_fudan,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_read_success =
        {
            .on_enter = nfc_scene_read_success_on_enter_fudan,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_saved_menu =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_save_name =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_emulate =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
    .scene_write =
        {
            .on_enter = nfc_protocol_support_common_on_enter_empty,
            .on_event = nfc_protocol_support_common_on_event_empty,
        },
};

NFC_PROTOCOL_SUPPORT_PLUGIN(fudan_fm11rf005, NfcProtocolFudanFm11rf005);
