#include "include/mem.h"
#include <stdio.h>
#include <string.h>

str *asf_mem_get_str(struct asf_mem *mem)
{
	str *s = str_new();
	const char *temp = "(%%%s)";
	const char *temp_offset = "%d(%%%s)";
	if (mem->offset) {
		str_expand(s, strlen(temp_offset) + sllen(mem->offset));
		snprintf(s->s, s->len, temp_offset,
				mem->offset,
				asf_regs[mem->addr].name);
		return s;
	}
	str_expand(s, strlen(temp) + 1);
	snprintf(s->s, s->len, temp, asf_regs[mem->addr].name);
	return s;
}
