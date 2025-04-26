#ifndef AMC_BE_ASF_JMP_H
#define AMC_BE_ASF_JMP_H

enum ASF_JMP_TYPE {
	ASF_JMP_EQ,
	ASF_JMP_GE,
	ASF_JMP_GT,
	ASF_JMP_LE,
	ASF_JMP_LT,
	ASF_JMP_NE
};

int asf_inst_jmp(enum ASF_JMP_TYPE inst, const char *label, int label_len);

#endif
