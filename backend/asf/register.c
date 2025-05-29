#include "include/register.h"
#include "include/imm.h"
#include "include/mov.h"
#include "../../include/backend.h"

struct asf_reg asf_regs[] = {
	[ASF_REG_RAX] = {"rax", 8, {1, 1}, NULL},
	[ASF_REG_RBX] = {"rbx", 8, {1, 1}, NULL},
	[ASF_REG_RCX] = {"rcx", 8, {1, 1}, NULL},
	[ASF_REG_RDX] = {"rdx", 8, {1, 1}, NULL},
	[ASF_REG_RBP] = {"rbp", 8, {1, 0}, NULL},
	[ASF_REG_RSI] = {"rsi", 8, {1, 1}, NULL},
	[ASF_REG_RDI] = {"rdi", 8, {1, 1}, NULL},
	[ASF_REG_RSP] = {"rsp", 8, {1, 0}, NULL},

	// only in 64-bit
	[ASF_REG_R8]  = {"r8",  8, {1, 1}, NULL},
	[ASF_REG_R9]  = {"r9",  8, {1, 1}, NULL},
	[ASF_REG_R10] = {"r10", 8, {1, 1}, NULL},
	[ASF_REG_R11] = {"r11", 8, {1, 1}, NULL},
	[ASF_REG_R12] = {"r12", 8, {1, 1}, NULL},
	[ASF_REG_R13] = {"r13", 8, {1, 1}, NULL},
	[ASF_REG_R14] = {"r14", 8, {1, 1}, NULL},
	[ASF_REG_R15] = {"r15", 8, {1, 1}, NULL},

	[ASF_REG_EAX] = {"eax", 4, {0, 1}, NULL},
	[ASF_REG_EBX] = {"ebx", 4, {0, 0}, NULL},
	[ASF_REG_ECX] = {"ecx", 4, {0, 1}, NULL},
	[ASF_REG_EDX] = {"edx", 4, {0, 1}, NULL},
	[ASF_REG_EBP] = {"ebp", 4, {0, 0}, NULL},
	[ASF_REG_ESI] = {"esi", 4, {0, 0}, NULL},
	[ASF_REG_EDI] = {"edi", 4, {0, 0}, NULL},
	[ASF_REG_ESP] = {"esp", 4, {0, 0}, NULL},
};

str *asf_reg_clean(enum ASF_REGS reg)
{
	struct asf_imm zero = {.iq = 0, .type = asf_regs[reg].bytes};
	*asf_regs[reg].purpose = ASF_REG_PURPOSE_NULL;
	return asf_inst_mov(ASF_MOV_I2R, &zero, &reg);
}

str *asf_reg_get_str(struct asf_reg *reg)
{
	str *s = str_new();
	str_expand(s, strlen(reg->name) + 2);
	s->s[0] = '%';
	memcpy(&s->s[1], reg->name, s->len - 1);
	return s;
}

enum ASF_REGS asf_reg_get(enum ASF_BYTES bytes)
{
	switch (bytes) {
	case ASF_BYTES_8:
	case ASF_BYTES_U8:
	case ASF_BYTES_16:
	case ASF_BYTES_U16:
	case ASF_BYTES_32:
	case ASF_BYTES_U32:
		return ASF_REG_EAX;
		break;
	case ASF_BYTES_64:
	case ASF_BYTES_U64:
		return ASF_REG_RAX;
		break;
	default:
		printf("amc[backend.asf]: asf_reg_get: "
				"Unsupport type: \"%d\"\n",
				bytes);
		backend_stop(BE_STOP_SIGNAL_ERR);
		break;
	}

	return -1;
}

int asf_regs_init()
{
	enum ASF_REG_PURPOSE_TYPE *purposes = calloc(ASF_REG_R15,
			sizeof(*purposes));
	for (int i = 0; i < ASF_REG_R15; i++) {
		asf_regs[i].purpose = &purposes[i];
	}
	for (int i = ASF_REG_EAX, base_reg = ASF_REG_RAX;
			i < LENGTH(asf_regs); i++) {
		asf_regs[i].purpose = asf_regs[base_reg].purpose;
		if (base_reg > ASF_REG_RSP)
			base_reg = ASF_REG_RAX;
		base_reg++;
	}
	return 0;
}
