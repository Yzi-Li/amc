#include "../include/backend.h"
#include "../include/config.h"

/* built backends import */
#ifdef AMC_BUILT_BACKEND
#include "asf/asf.h"
#endif

enum FLAG {
	FLAG_INITED = 1 << 0,
	FLAG_STOPED = 1 << 1,
	FLAG_ENDED  = 1 << 2,
};

static int flag = 0;

struct backend *backends[] = {
	NULL,
	&backend_asf
};

enum BACKENDS cur_backend;

int backend_init(int argc, char *argv[])
{
	if (flag & FLAG_INITED)
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
	if (flag & FLAG_STOPED)
		return 0;
	return backends[cur_backend]->stop(bess);
}

int backend_end()
{
	if (flag & FLAG_ENDED)
		return 0;
	return backends[cur_backend]->end();
}
