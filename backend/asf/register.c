#include "register.h"
#include "inst.h"
#include "../../include/backend.h"

struct asf_reg regs[] = {
	[ASF_REG_RAX] = {"rax", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_RBX] = {"rbx", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_RCX] = {"rcx", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_RDX] = {"rdx", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_RBP] = {"rbp", 8, {1, 0, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_RSI] = {"rsi", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_RDI] = {"rdi", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_RSP] = {"rsp", 8, {1, 0, 0}, ASF_REG_PURPOSE_NULL},

	// only in 64-bit
	[ASF_REG_R8]  = {"r8",  8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_R9]  = {"r9",  8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_R10] = {"r10", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_R11] = {"r11", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_R12] = {"r12", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_R13] = {"r13", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_R14] = {"r14", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_R15] = {"r15", 8, {1, 1, 0}, ASF_REG_PURPOSE_NULL},

	[ASF_REG_EAX] = {"eax", 4, {0, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_EBX] = {"ebx", 4, {0, 0, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_ECX] = {"ecx", 4, {0, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_EDX] = {"edx", 4, {0, 1, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_EBP] = {"ebp", 4, {0, 0, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_ESI] = {"esi", 4, {0, 0, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_EDI] = {"edi", 4, {0, 0, 0}, ASF_REG_PURPOSE_NULL},
	[ASF_REG_ESP] = {"esp", 4, {0, 0, 0}, ASF_REG_PURPOSE_NULL},
};

str *asf_reg_clean(struct asf_reg *reg)
{
	struct asf_imm zero = {.iq = 0, .type = reg->size};
	reg->purpose = ASF_REG_PURPOSE_NULL;
	reg->flags.used = 0;
	return asf_inst_mov(ASF_MOV_I2R, &zero, reg);
}

char *asf_reg_get_chr(struct asf_reg *reg)
{
	char *result = NULL;
	str *s = asf_reg_get_str(reg);
	result = s->s;
	free(s);
	return result;
}

str *asf_reg_get_str(struct asf_reg *reg)
{
	str *s = str_new();
	str_expand(s, strlen(reg->name) + 1);
	snprintf(s->s, s->len, "%%%s", reg->name);
	return s;
}

enum ASF_REGS asf_reg_get(enum YZ_TYPE type)
{
	switch (yz_type_table[type - AM_TYPE_OFFSET].len) {
	case 1:
	case 2:
	case 4:
		return ASF_REG_EAX;
		break;
	case 8:
		return ASF_REG_RAX;
		break;
	default:
		printf("amc: reg_get: unsupport type");
		backend_stop(BE_STOP_SIGNAL_ERR);
		break;
	}

	return -1;
}
