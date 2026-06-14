#include "fudan_fm11rf005_render.h"

void nfc_render_fudan_fm11rf005_info(
    const FudanFm11rf005Data* data,
    NfcProtocolFormatType format_type,
    FuriString* str) {
    furi_string_cat_printf(str, "UID:");

    for(size_t i = 0; i < FUDAN_FM11RF005_UID_SIZE; i++) {
        furi_string_cat_printf(str, " %02X", data->uid[i]);
    }

    furi_string_cat_printf(
        str, "\nATQA: %02X %02X", (uint8_t)(data->atqa >> 8), (uint8_t)(data->atqa & 0xFF));
    furi_string_cat_printf(str, "\nSAK: 0x%02X\n", data->sak);

    if(format_type == NfcProtocolFormatTypeFull) {
        furi_string_cat_printf(
            str,
            "\nCID: %02X %02X\nMID: %02X %02X",
            data->pages[0][0],
            data->pages[0][1],
            data->pages[0][2],
            data->pages[0][3]);

        furi_string_cat_printf(str, "\n------[Pages]------");
        for(size_t i = 0; i < FUDAN_FM11RF005_PAGE_NUM; i++) {
            furi_string_cat_printf(
                str,
                "\n p%02X :  %02X %02X %02X %02X",
                (uint8_t)i,
                data->pages[i][0],
                data->pages[i][1],
                data->pages[i][2],
                data->pages[i][3]);
        }
    }
}
