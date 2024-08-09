#ifndef AMC_EXPR_H
#define AMC_EXPR_H
#include "../include/symbol.h"
#include "../include/type.h"
#include "../include/file.h"
#include "../include/backend/operator.h"

struct expr_operator {
	const char *sym;
	int priority;
	enum OP_ID id;
};

struct expr {
	yz_val *vall, *valr;
	struct expr_operator *op;
};

struct expr_eval_queue {
	struct expr *queue;
	u8 cur, len;
};

int parse_assignment_expr(yz_val *l, yz_val *r);

// some operator need to be parsed in frontend first
static struct expr_operator operators[] = {
	{"*",   1, OP_MUL       },
	{"/",   1, OP_DIV       },
	{"+",   2, OP_ADD       },
	{"-",   2, OP_SUB       },

	{"==",  3, OP_EQ        },
	{"!=",  3, OP_NE        },
	{"<",   3, OP_LT        },
	{"<=",  3, OP_LE        },
	{">",   3, OP_GT        },
	{">=",  3, OP_GE        },

	{"and", 4, OP_AND       },
	{"or",  4, OP_OR        },

	{"=",   5, OP_ASSIGNMENT},
	{NULL,  0, OP_NONE      }
};

static backend_op_cmd_f speical_ops[] = {
	parse_assignment_expr
};

static const struct expr_operator unary_ops[] = {
	{"!",   3, OP_NOT },
	//{"-",   3, backend_
	{NULL,  0, OP_NONE}
};

yz_val *expr_apply(struct expr *e);
void expr_free(struct expr *e);
struct expr *parse_expr(struct file *f, int top);

#endif
