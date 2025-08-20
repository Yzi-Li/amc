/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/backend.h"
#include "../include/checker/type.h"

int check_type_equal(yz_type *src, yz_type *dest)
{
	if (yz_type_max(src, dest) == NULL)
		goto err_wrong_type;
	return 1;
err_wrong_type:
	printf("amc: check_type_equal: ERROR: Wrong type!\n"
			"| HINT: Symbol type: \"%s\"\n"
			"|       Value type:  \"%s\"\n",
			yz_get_type_name(dest),
			yz_get_type_name(src));
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 0;
}
