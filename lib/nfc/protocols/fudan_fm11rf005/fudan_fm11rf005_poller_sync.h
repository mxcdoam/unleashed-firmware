#pragma once

#include "fudan_fm11rf005.h"
#include <nfc/nfc.h>
#include <nfc/nfc_poller.h>

#ifdef __cplusplus
extern "C" {
#endif

FudanFm11rf005Error
    fudan_fm11rf005_poller_sync_read_all(Nfc* nfc, FudanFm11rf005Data* data);

#ifdef __cplusplus
}
#endif
