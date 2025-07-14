#include "include/asf.h"

int asf_symbol_get_path(str *result, str *mod, const char *name, int name_len)
{
	char *base = mod->s;
	int count = 0;
	for (int i = 0; i < mod->len; i++) {
		if (mod->s[i] != '/') {
			count++;
			continue;
		}
		str_append(result, count, base);
		str_append(result, 2, "__");
		i++;
		base = &mod->s[i];
		count = 0;
	}
	str_append(result, count, base);
	str_append(result, 2, "__");
	str_append(result, name_len, name);
	str_append(result, 1, "\0");
	return 0;
}

void asf_symbol_status_free(backend_symbol_status *raw_stat)
{
	return;
}
