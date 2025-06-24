#include "../../include/backend.h"
#include "../../include/comptime/type.h"

int comptime_type_check_equal(yz_val *src, yz_val *dest)
{
	yz_val *val = NULL;
	if ((val = yz_type_max(src, dest)) == NULL)
		goto err_wrong_type;
	return 0;
err_wrong_type:
	printf("amc: comptime_type_check_equal: ERROR: Wrong type!\n"
			"| HINT: Symbol type: \"%s\"\n"
			"|       Value type:  \"%s\"\n",
			yz_get_type_name(dest),
			yz_get_type_name(src));
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}
