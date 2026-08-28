/* elsytools.c - Utilities for parsing ElsY card document data.
 */

#include "elsytools.h"

#include <flipper_format/flipper_format.h>

#define ELSY_NOT_SET          "Not Set"
#define ELSY_CODE_KEY_LEN     4
#define ELSY_FIELD_MAX        32
#define ELSY_INITIALS_MAX     8
#define ELSY_REGION_CODE_FILE EXT_PATH("nfc/assets/ru_region_id.nfc")

/* cp1251 byte -> uppercase Latin string lookup table.
 * Non-Cyrillic ASCII bytes map to themselves, so digits, dot, space, comma,
 * 'F' and 'M' pass through unchanged. Unknown bytes are dropped.
 */
static const char* const CP1251_TO_LATIN_UPPER[256] = {
    /* 0x00-0x1F */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    /* 0x20-0x3F */
    " ",
    "!",
    "\"",
    "#",
    "$",
    "%",
    "&",
    "'",
    "(",
    ")",
    "*",
    "+",
    ",",
    "-",
    ".",
    "/",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    ":",
    ";",
    "<",
    "=",
    ">",
    "?",
    /* 0x40-0x5F */
    "@",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "[",
    "\\",
    "]",
    "^",
    "_",
    /* 0x60-0x7F */
    "`",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "{",
    "|",
    "}",
    "~",
    "",
    /* 0x80-0x8F */
    "D",
    "G",
    "",
    "G",
    "",
    "",
    "",
    "",
    "",
    "",
    "LJ",
    "",
    "NJ",
    "K",
    "CH",
    "DZ",
    /* 0x90-0x9F */
    "D",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "LJ",
    "",
    "NJ",
    "K",
    "CH",
    "DZ",
    "I",
    /* 0xA0-0xAF */
    " ",
    "U",
    "U",
    "J",
    "",
    "G",
    "",
    "",
    "YO",
    "",
    "YE",
    "",
    "",
    "",
    "",
    "YI",
    /* 0xB0-0xBF */
    "",
    "",
    "I",
    "I",
    "G",
    "",
    "",
    "",
    "YO",
    "",
    "YE",
    "",
    "J",
    "S",
    "S",
    "YI",
    /* 0xC0-0xDF: А Б В Г Д Е Ж З И Й К Л М Н О П Р С Т У Ф Х Ц Ч Ш Щ Ъ Ы Ь Э Ю Я */
    "A",
    "B",
    "V",
    "G",
    "D",
    "E",
    "ZH",
    "Z",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "R",
    "S",
    "T",
    "U",
    "F",
    "H",
    "C",
    "CH",
    "SH",
    "SCH",
    "\"",
    "Y",
    "'",
    "E",
    "YU",
    "YA",
    /* 0xE0-0xFF: а б в г д е ж з и й к л м н о п р с т у ф х ц ч ш щ ъ ы ь э ю я */
    "A",
    "B",
    "V",
    "G",
    "D",
    "E",
    "ZH",
    "Z",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "R",
    "S",
    "T",
    "U",
    "F",
    "H",
    "C",
    "CH",
    "SH",
    "SCH",
    "\"",
    "Y",
    "'",
    "E",
    "YU",
    "YA",
};

static bool byte_is_digit(const uint8_t c) {
    return c >= '0' && c <= '9';
}

static bool byte_is_letter(const uint8_t c) {
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c >= 0xC0;
}

static void elsy_transliterate(const uint8_t* data, FuriString* out, bool skip_separators) {
    furi_string_reset(out);
    const size_t max = skip_separators ? ELSY_INITIALS_MAX : ELSY_FIELD_MAX;
    for(size_t i = 0; i < max; i++) {
        const uint8_t c = data[i];
        if(c == 0x00) {
            if(!skip_separators) break;
            continue;
        }
        if(c == 0x20) continue;
        furi_string_cat(out, CP1251_TO_LATIN_UPPER[c]);
    }
    if(furi_string_size(out) == 0) furi_string_set(out, ELSY_NOT_SET);
}

static void elsy_lookup_region(const char* key, FuriString* region) {
    Storage* storage = furi_record_open(RECORD_STORAGE);
    FlipperFormat* file = flipper_format_file_alloc(storage);
    if(flipper_format_file_open_existing(file, ELSY_REGION_CODE_FILE)) {
        flipper_format_read_string(file, key, region);
    }
    flipper_format_free(file);
    furi_record_close(RECORD_STORAGE);
}

void get_elsy_doc(
    const uint8_t* id_info,
    const uint8_t* surname,
    const uint8_t* name,
    const uint8_t* patronymic,
    uint8_t* id_type,
    FuriString* region,
    FuriString* id_series,
    FuriString* id_number,
    FuriString* surname_out,
    FuriString* name_out,
    FuriString* patronymic_out) {
    if(id_info != NULL) {
        const bool digit1 = byte_is_digit(id_info[3]);
        const bool digit2 = byte_is_digit(id_info[4]);
        const bool letter1 = byte_is_letter(id_info[3]);
        const bool letter2 = byte_is_letter(id_info[4]);

        uint8_t type = ELSY_DOC_UNDEF;
        if(digit1 && digit2) {
            type = ELSY_DOC_PASSPORT;
        } else if(letter1 && letter2) {
            if(id_info[0] == '0' && id_info[1] == '0') {
                type = ELSY_DOC_FOREIGN_PASSPORT;
            } else {
                type = ELSY_DOC_BIRTH_CERT;
            }
        }
        if(id_type != NULL) *id_type = type;

        if(region != NULL && type == ELSY_DOC_PASSPORT) {
            char key[ELSY_CODE_KEY_LEN + 1] = {0};
            snprintf(key, sizeof(key), "%02X%02X", id_info[0], id_info[1]);
            furi_string_reset(region);
            elsy_lookup_region(key, region);
            if(furi_string_size(region) == 0) {
                furi_string_set(region, ELSY_NOT_SET);
            }
        }

        if(id_series != NULL) {
            furi_string_reset(id_series);
            furi_string_cat(id_series, CP1251_TO_LATIN_UPPER[id_info[0]]);
            furi_string_cat(id_series, CP1251_TO_LATIN_UPPER[id_info[1]]);
            furi_string_cat(id_series, " ");
            furi_string_cat(id_series, CP1251_TO_LATIN_UPPER[id_info[3]]);
            furi_string_cat(id_series, CP1251_TO_LATIN_UPPER[id_info[4]]);
        }

        if(id_number != NULL) {
            const uint32_t num = (uint32_t)id_info[5] | ((uint32_t)id_info[6] << 8) |
                                 ((uint32_t)id_info[7] << 16);
            furi_string_printf(id_number, "%06lu", (unsigned long)num);
        }
    }

    if(surname != NULL && surname_out != NULL) {
        elsy_transliterate(surname, surname_out, false);
    }

    if(name != NULL && name_out != NULL) {
        elsy_transliterate(name, name_out, patronymic == NULL);
    }

    if(patronymic != NULL && patronymic_out != NULL) {
        elsy_transliterate(patronymic, patronymic_out, false);
    }
}
