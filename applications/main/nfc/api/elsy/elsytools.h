/* elsytools.h - Utilities for parsing ElsY card document data.
 */

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <furi/core/string.h>
#include <storage/storage.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ELSY_DOC_UNDEF 0
#define ELSY_DOC_BIRTH_CERT 1
#define ELSY_DOC_PASSPORT 2
#define ELSY_DOC_FOREIGN_PASSPORT 3

/* Parses the ElsY card document info block and personal data fields.
 *
 * id_info points to an 8-byte field laid out as:
 *   [0] cp1251 digit, [1] cp1251 digit, [2] 0x20 space, [3] digit or letter,
 *   [4] digit or letter, [5..7] three bytes of little-endian hex.
 *
 * id_type is set to ELSY_DOC_BIRTH_CERT when bytes [3] and [4] are letters,
 * ELSY_DOC_PASSPORT when they are digits, otherwise ELSY_DOC_UNDEF.
 *
 * For a passport, region is filled by looking up the first two digit bytes
 * ([0], [1]) in the hardcoded region code table file; it is left untouched
 * otherwise and set to "Not Set" when the code is not found.
 *
 * id_series is set to "dd <symbols>" where dd are bytes [0] and [1] and
 * <symbols> are bytes [3] and [4] transliterated to uppercase Latin.
 *
 * id_number is set to the decimal value of the three little-endian bytes.
 *
 * surname, name and patronymic point to cp1251 encoded byte strings and are
 * transliterated to uppercase Latin into surname_out, name_out and
 * patronymic_out respectively. Each input may be NULL, in which case the
 * corresponding output is left untouched. When patronymic is NULL, name is
 * treated as an initials field: 0x00 and 0x20 bytes inside it are skipped
 * and only the meaningful symbols are kept.
 *
 * Any output pointer (id_type, region, id_series, id_number, surname_out,
 * name_out, patronymic_out) may be NULL and the corresponding step is skipped.
 */
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
    FuriString* patronymic_out);

#ifdef __cplusplus
}
#endif
