#include "imm.h"
#include "inst.h"
#include "register.h"
#include "stack.h"
#include "suffix.h"
#include <stdio.h>

static const struct inst insts[] = {
	{"pop%c %s\n" },
	{"push%c %s\n"},
};

str *asf_inst_pop(enum ASF_IMM_TYPE bytes, const char *dest)
{
	str *s = str_new();
	str_expand(s, strlen(insts[0].code) - 2);
	snprintf(s->s, s->len, insts[0].code,
			suffix_get(bytes),
			dest);
	return s;
}

str *asf_inst_push(enum ASF_IMM_TYPE bytes, const char *src)
{
	str *s = str_new();
	str_expand(s, (strlen(insts[1].code) - 2) + strlen(src));
	snprintf(s->s, s->len, insts[1].code,
			suffix_get(bytes),
			src);
	return s;
}

str *asf_inst_pushi(struct asf_imm *imm)
{
	str *s = str_new();
	str *tmp = asf_imm_str_new(imm);
	str_expand(s, strlen(insts[1].code) - 2);
	snprintf(s->s, s->len, insts[1].code,
			suffix_get(imm->type),
			tmp->s);
	return s;
}
