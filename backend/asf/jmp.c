#include "include/jmp.h"
#include "../../utils/str/str.h"
#include "../../utils/utils.h"
#include <stdio.h>

static const char *insts[] = {
	[ASF_JMP_ALWAYS] = "jmp %s\n",
	[ASF_JMP_EQ]     = "je %s\n",
	[ASF_JMP_GE]     = "jge %s\n",
	[ASF_JMP_GT]     = "jg %s\n",
	[ASF_JMP_LE]     = "jle %s\n",
	[ASF_JMP_LT]     = "jl %s\n",
	[ASF_JMP_NE]     = "jne %s\n"
};

str *asf_inst_jmp(enum ASF_JMP_TYPE inst, const char *label, int label_len)
{
	str *s = NULL;
	const char *temp = NULL;
	if (inst > LENGTH(insts))
		return NULL;
	temp = insts[inst];
	s = str_new();
	str_expand(s, strlen(temp) - 1 + label_len);
	snprintf(s->s, s->len, temp, label);
	return s;
}
