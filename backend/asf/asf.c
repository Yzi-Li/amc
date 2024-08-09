#include "asf.h"
#include "../../include/backend.h"
#include "../../include/backend/target.h"

int asf_init(int argc, char *argv[])
{
	objs = malloc(sizeof(void*));
	return 0;
}

int asf_file_new(struct file *f)
{
	cur_obj++;
	objs[cur_obj] = calloc(3, sizeof(void*));
	return 0;
}

int asf_stop(enum BE_STOP_SIGNAL bess)
{
	switch (bess) {
	case BE_STOP_SIGNAL_NULL:
		//break;
	case BE_STOP_SIGNAL_ERR:
	default:
		object_free(objs[cur_obj]);
		break;
	}

	return 0;
}

int asf_end()
{
	target_write(objs, 1);
	return 0;
}
