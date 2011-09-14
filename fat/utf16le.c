/* The FreeDOS-32 Unicode Support Library version 2.1
 * Copyright (C) 2001-2006  Salvatore ISAJA
 *
 * This file "utf16le.c" is part of the FreeDOS-32 Unicode
 * Support Library (the Program).
 *
 * The Program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * The Program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with the Program; see the file GPL.txt; if not, write to
 * the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include <config.h>
#include "unicode.h"
#include <endian.h>

/**
 * \addtogroup unicode
 * @{
 */

static inline uint16_t little_endian_16(uint16_t v)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN /* Machine is little endian */
	return v;
#else /* Machine is big endian */
	return (v << 8) | (v >> 8);
#endif
}


/**
 * \brief Gets the length of a UTF-16LE character.
 * \param lead_word the first uint16_t of a UTF-16 character;
 * \retval >0 the length in uint16_t units of the UTF-16 character;
 * \remarks For performance reasons, this function does not parse
 *          the whole UTF-16 word sequence, just the first uint16_t.
 *          If checking the validity of the whole UTF-16 word sequence
 *          is needed, use unicode_utf16le_to_wchar().
 * \remarks This is the little endian version of unicode_utf16be_len()
 */
int unicode_utf16le_len(uint16_t lead_word)
{
	if ((little_endian_16(lead_word) & 0xFC00) == 0xD800) return 2;
	return 1;
}


/**
 * \brief UTF-16LE to wide character.
 * \param result where to store the converted wide character;
 * \param string buffer containing the UTF-16 character to convert;
 * \param size max number of uint16_t units of \c string to examine;
 * \retval >0 the length in uint16_t units of the processed UTF-16
 *            character, the wide character is stored in \c result;
 * \retval -EILSEQ invalid UTF-16 word sequence;
 * \retval -ENAMETOOLONG \c size too small to parse the UTF-16 character.
 * \remarks This is the little endian version of unicode_utf16be_to_wchar()
 */
int unicode_utf16le_to_wchar(wchar_t *restrict result, const uint16_t *restrict string, size_t size)
{
	uint16_t v;
	if (!size) return -ENAMETOOLONG;
	v = little_endian_16(*string);
	if ((v & 0xFC00) != 0xD800)
	{
		*result = (wchar_t) v;
		return 1;
	}
	if (size < 2) return -ENAMETOOLONG;
	*result = ((v & 0x03FF) << 10) + 0x010000;
	v = little_endian_16(*(++string));
	if ((v & 0xFC00) != 0xDC00) return -EILSEQ;
	*result |= v & 0x03FF;
	return 2;
}


/**
 * \brief Wide character to UTF-16.
 * \param s where to store the converted UTF-16 character;
 * \param wc the wide character to convert;
 * \param size max number of uint16_t units to store in \c s;
 * \retval >0 the length in uint16_t units of the converted UTF-16 character, stored in \c s;
 * \retval -EINVAL invalid wide character (don't know how to convert it to UTF-16);
 * \retval -ENAMETOOLONG \c size too small to store the UTF-16 character.
 * \remarks This is the little endian version of unicode_wchar_to_utf16be()
 */
int unicode_wchar_to_utf16le(uint16_t *s, wchar_t wc, size_t size)
{
	if (wc >= 0)
	{
		if (wc < 0x010000)
		{
			*s = little_endian_16((uint16_t) wc);
			return 1;
		}
		if (wc < 0x200000)
		{
			*s       = little_endian_16((uint16_t) (0xD800 + (((wc >> 16) - 1) << 6) + ((wc & 0x00FC00) >> 2)));
			*(s + 1) = little_endian_16((uint16_t) (0xDC00 + (wc & 0x0003FF)));
			return 2;
		}
	}
	return -EINVAL;
}

/* @} */
