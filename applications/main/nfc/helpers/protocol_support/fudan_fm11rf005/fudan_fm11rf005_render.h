#pragma once

#include <nfc/protocols/fudan_fm11rf005/fudan_fm11rf005.h>

#include "../nfc_protocol_support_render_common.h"

void nfc_render_fudan_fm11rf005_info(
    const FudanFm11rf005Data* data,
    NfcProtocolFormatType format_type,
    FuriString* str);
