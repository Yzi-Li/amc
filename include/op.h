/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_OP_H
#define AMC_OP_H

enum OP_ID {
	OP_ADD,
	OP_DIV,
	OP_MUL,
	OP_SUB,

	OP_EQ,
	OP_NE,
	OP_LE,
	OP_LT,
	OP_GE,
	OP_GT,

	OP_AND,
	OP_OR,
	OP_NOT,

	OP_ASSIGN,
	OP_ASSIGN_ADD,
	OP_ASSIGN_DIV,
	OP_ASSIGN_MUL,
	OP_ASSIGN_SUB,

	OP_EXTRACT_VAL,
	OP_GET_ADDR,

	OP_LEN
};

#define OP_SPECIAL_START 13

#define OP_IS_CMP(ID) (REGION_INT(ID, OP_EQ, OP_GT))

#endif
