/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/suffix.h"

char asf_suffix_get(enum ASF_BYTES bytes)
{
	switch (bytes) {
	case ASF_BYTES_8:
	case ASF_BYTES_U8:
		return 'b';
		break;
	case ASF_BYTES_16:
	case ASF_BYTES_U16:
		return 'w';
		break;
	case ASF_BYTES_32:
	case ASF_BYTES_U32:
		return 'l';
		break;
	case ASF_BYTES_64:
	case ASF_BYTES_U64:
		return 'q';
		break;
	default:
		return 'E';
		break;
	}

	return '\0';
}
