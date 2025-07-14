#ifndef AMC_BE_SYMBOL_H
#define AMC_BE_SYMBOL_H
#include "../../utils/str/str.h"

typedef void backend_symbol_status;

typedef int (*backend_symbol_get_path_f)(str *result, str *mod,
		const char *name, int name_len);
typedef void (*backend_symbol_status_free_f)(backend_symbol_status *raw_stat);

#endif
