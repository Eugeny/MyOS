/* The FreeDOS-32 Unicode Support Library version 2.1
 * Copyright (C) 2001-2006  Salvatore ISAJA
 *
 * This file "utf8.c" is part of the FreeDOS-32 Unicode
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

/**
 * \addtogroup unicode
 * @{
 */

/* Bit mask and bit values of a UTF-8 character lead byte */
static struct { char mask; char val; } t[4] =
{ { 0x80, 0x00 }, { 0xE0, 0xC0 }, { 0xF0, 0xE0 }, { 0xF8, 0xF0 } };


/**
 * \brief Gets the length of a UTF-8 character.
 * \param lead_byte the first byte of a UTF-8 character;
 * \retval >0 the length in bytes of the UTF-8 character;
 * \retval -EILSEQ invalid UTF-8 lead byte;
 * \remarks For performance reasons, this function does not parse
 *          the whole UTF-8 byte sequence, just the first byte.
 *          If checking the validity of the whole UTF-8 byte sequence
 *          is needed, use unicode_utf8_to_wchar().
 */
int unicode_utf8_len(char lead_byte)
{
	int k;
	for (k = 0; k < 4; k++)
		if ((lead_byte & t[k].mask) == t[k].val)
			return k + 1;
	return -EILSEQ;
}


/**
 * \brief UTF-8 to wide character.
 * \param result where to store the converted wide character;
 * \param string buffer containing the UTF-8 character to convert;
 * \param size max number of bytes of \c string to examine;
 * \retval >0 the length in bytes of the processed UTF-8 character, the wide character is stored in \c result;
 * \retval -EILSEQ invalid UTF-8 byte sequence;
 * \retval -ENAMETOOLONG \c size too small to parse the UTF-8 character.
 */
int unicode_utf8_to_wchar(wchar_t *restrict result, const char *restrict string, size_t size)
{
	wchar_t wc = 0;
	unsigned k, j;
	if (!size) return -ENAMETOOLONG;
	for (k = 0; k < 4; k++)
		if ((*string & t[k].mask) == t[k].val)
		{
			if (size < k + 1) return -ENAMETOOLONG;
			wc = (wchar_t) (unsigned char) *string & ~t[k].mask;
			for (j = 0; j < k; j++)
			{
				if ((*(++string) & 0xC0) != 0x80) return -EILSEQ;
				wc = (wc << 6) | ((wchar_t) (unsigned char) *string & 0x3F);
			}
			*result = wc;
			return k + 1;
		}
	return -EILSEQ;
}

/**
 * \brief Wide character to UTF-8.
 * \param s where to store the converted UTF-8 character;
 * \param wc the wide character to convert;
 * \param size max number of bytes to store in \c s;
 * \retval >0 the length in bytes of the converted UTF-8 character, stored in \c s;
 * \retval -EINVAL invalid wide character (don't know how to convert it to UTF-8);
 * \retval -ENAMETOOLONG \c size too small to store the UTF-8 character.
 */
int unicode_wchar_to_utf8(char *s, wchar_t wc, size_t size)
{
	if (wc >= 0)
	{
		if (wc < 0x000080)
		{
			if (size < 1) return -ENAMETOOLONG;
			*s = (char) wc;
			return 1;
		}
		if (wc < 0x000800)
		{
			if (size < 2) return -ENAMETOOLONG;
			*(s + 1) = (char) (0x80 | (wc & 0x3F)); wc >>= 6;
			*s = (char) (0xC0 | wc);
			return 2;
		}
		if (wc < 0x010000)
		{
			if (size < 3) return -ENAMETOOLONG;
			*(s + 2) = (char) (0x80 | (wc & 0x3F)); wc >>= 6;
			*(s + 1) = (char) (0x80 | (wc & 0x3F)); wc >>= 6;
			*s = (char) (0xE0 | wc);
			return 3;
		}
		if (wc < 0x200000)
		{
			if (size < 4) return -ENAMETOOLONG;
			*(s + 3) = (char) (0x80 | (wc & 0x3F)); wc >>= 6;
			*(s + 2) = (char) (0x80 | (wc & 0x3F)); wc >>= 6;
			*(s + 1) = (char) (0x80 | (wc & 0x3F)); wc >>= 6;
			*s = (char) (0xF0 | wc);
			return 4;
		}
	}
	return -EINVAL;
}

/* @} */
