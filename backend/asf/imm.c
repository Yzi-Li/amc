#include "include/imm.h"
#include "../../utils/utils.h"
#include <stdio.h>

str *asf_imm_str_new(struct asf_imm *imm)
{
	str *s = str_new();
	str_expand(s, ullen(imm->iq) + 2);
	snprintf(s->s, s->len, "$%lld", imm->iq);
	return s;
}
