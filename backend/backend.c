#include "../include/backend.h"
#include "../include/config.h"

/* built backends import */
#ifdef AMC_BUILT_BACKEND
#include "asf/include/asf.h"
#endif

extern struct backend backend_asf;

struct backend *backends[] = {
	NULL,
	&backend_asf
};

int backend_flag = 0;
enum BACKENDS cur_backend;

int backend_init(int argc, char *argv[])
{
	if (backend_flag & BE_FLAG_INITED)
		return 0;
	if (cur_backend == BE_NONE)
		cur_backend = default_backend;
	return backends[cur_backend]->init(argc, argv);
}

int backend_file_new(struct file *f)
{
	return backends[cur_backend]->file_new(f);
}

int backend_stop(enum BE_STOP_SIGNAL bess)
{
	if (backend_flag & BE_FLAG_STOPED)
		return 0;
	if (backends[cur_backend]->stop(bess))
		return 1;
	backend_flag |= BE_FLAG_STOPED;
	return 0;
}

int backend_end()
{
	if (backend_flag & BE_FLAG_ENDED)
		return 0;
	return backends[cur_backend]->end();
}
