/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/backend.h"
#include "../include/checker/type.h"

int check_type_equal(yz_type *src, yz_type *dest)
{
	if (src->type == YZ_ENUM && dest->type != YZ_ENUM_ITEM)
		goto err_not_enum_item;
	if (src->type != YZ_ENUM_ITEM && dest->type == YZ_ENUM)
		goto err_not_enum_item;
	if (yz_type_max(src, dest) == NULL)
		goto err_wrong_type;
	return 1;
err_not_enum_item:
	printf("amc: %s: "ERROR_STR": Failed to compare digit with enum!\n",
			__func__);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 0;
err_wrong_type:
	printf("amc: %s: "ERROR_STR": Wrong type!\n"
			"| HINT: Symbol type: \"%s\"\n"
			"|       Value type:  \"%s\"\n",
			__func__,
			yz_get_type_name(dest),
			yz_get_type_name(src));
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 0;
}
