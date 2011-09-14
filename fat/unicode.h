/* Header file for the FreeDOS-32 Unicode Support Library version 2.1
 * by Salvo Isaja, 2005-2006
 */
#ifndef __FD32_UNICODE_H
#define __FD32_UNICODE_H

/**
\defgroup unicode Unicode support library

The FreeDOS-32 Unicode support library provides facilities to manage
UTF-8, UTF-16 and wide characters (format conversion, case folding).\n
This manual documents version <em>2.1</em> of the library.\n
The library is distributed under the terms of the GNU General Public License.

A UTF-8 character is converted to a wide character (UTF-32 or UCS-4)
using the following rules (binary numbers):
\code
UTF-32                     - UTF-8
00000000 00000000 0aaaaaaa - 0aaaaaaa
00000000 00000bbb bbaaaaaa - 110bbbbb 10aaaaaa
00000000 ccccbbbb bbaaaaaa - 1110cccc 10bbbbbb 10aaaaaa
000dddcc ccccbbbb bbaaaaaa - 11110ddd 10cccccc 10bbbbbb 10aaaaaa
\endcode

A UTF-16 character is converted to a wide character (UTF-32 or UCS-4)
using the following rules (binary numbers):
\code
UTF-32                     - UTF-16
00000000 aaaaaaaa aaaaaaaa <-> aaaaaaaa aaaaaaaa
000bbbbb aaaaaaaa aaaaaaaa <-> 110110cc ccaaaaaa  110111aa aaaaaaaa
\endcode
where \c cccc = \c bbbbb - 1.

@{ */

#include <config.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <linux/types.h>
#include <linux/unistd.h>
#include <sys/types.h>
#include <errno.h>

#define unicode_utf8len unicode_utf8_len ///< \deprecated Use unicode_utf8_len()
#define unicode_utf8towc unicode_utf8_to_wchar ///< \deprecated Use unicode_utf8_to_wchar()
#define unicode_wctoutf8 unicode_wchar_to_utf8 ///< \deprecated Use unicode_wchar_to_utf8()
#define unicode_utf16len unicode_utf16le_len ///< \deprecated Use unicode_utf16le_len()
#define unicode_utf16towc unicode_utf16le_to_wchar ///< \deprecated Use unicode_utf16le_to_wchar()
#define unicode_wctoutf16 unicode_wchar_to_utf16le ///< \deprecated Use unicode_wchar_to_utf16le()

int unicode_utf8_len(char lead_byte);
int unicode_utf8_to_wchar(wchar_t *restrict result, const char *restrict string, size_t size);
int unicode_wchar_to_utf8(char *s, wchar_t wc, size_t size);
int unicode_utf16le_len(uint16_t lead_word);
int unicode_utf16le_to_wchar(wchar_t *restrict result, const uint16_t *restrict string, size_t size);
int unicode_wchar_to_utf16le(uint16_t *s, wchar_t wc, size_t size);
int unicode_utf16be_len(uint16_t lead_word);
int unicode_utf16be_to_wchar(wchar_t *restrict result, const uint16_t *restrict string, size_t size);
int unicode_wchar_to_utf16be(uint16_t *s, wchar_t wc, size_t size);
wchar_t unicode_simple_fold(wchar_t wc);

/* @} */
#endif /* #ifndef __FD32_UNICODE_H */
