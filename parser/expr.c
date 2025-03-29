/**
 * TODO: 1. unary operator support.
 *       2. special operator support
 */
#include "expr.h"
#include "../include/backend.h"
#include "../include/token.h"
#include "../utils/converter.h"
#include "../utils/die.h"
#include "../utils/utils.h"
#include <string.h>

static const char *TERM_END = ";),] \t\n";

// some operator need to be parsed in frontend first
static const struct expr_operator operators[] = {
	{"*",   1, OP_MUL  },
	{"/",   1, OP_DIV  },
	{"+",   2, OP_ADD  },
	{"-",   2, OP_SUB  },

	{"==",  3, OP_EQ   },
	{"!=",  3, OP_NE   },
	{"<",   3, OP_LT   },
	{"<=",  3, OP_LE   },
	{">",   3, OP_GT   },
	{">=",  3, OP_GE   },

	{"and", 4, OP_AND  },
	{"or",  4, OP_OR   },

	{NULL,  0, OP_NONE }
};

static int expr_binary(struct file *f, struct expr *e, int top,
		struct scope *scope);
static int expr_check_end(char *endc, struct file *f, str *token, int top);
static enum YZ_TYPE *expr_get_sum_type(yz_val *l, yz_val *r);
static int expr_operator(struct file *f, struct expr_operator **op, int top);
static int expr_read_token(struct file *f, str *token, int top);
static int expr_sub(struct file *f, struct expr **e, int top,
		struct scope *scope);
static int expr_term(struct file *f, yz_val *v, int top, struct scope *scope);
static int expr_term_chr(struct file *f, yz_val *v, int top);
static int expr_term_expr(struct file *f, yz_val *v, int top,
		struct scope *scope);
static int expr_term_func(struct file *f, yz_val *v, int top,
		struct scope *scope);
static int expr_term_int(struct file *f, yz_val *v, int top);

int expr_binary(struct file *f, struct expr *e, int top, struct scope *scope)
{
	int ret = 0;
	if ((ret = expr_term(f, e->vall, top, scope)) > 0)
		return 1;
	if (ret == -1) {
		if (e->vall->type == AMC_EXPR) {
			e->sum_type = ((struct expr*)e->vall->v)->sum_type;
		} else {
			e->sum_type = &e->vall->type;
		}
		free_safe(e->valr);
		return -1;
	}

	if ((ret = expr_operator(f, &e->op, top)) > 0)
		return 1;
	if (ret == -1)
		goto err_valr_not_found;
	if (!top)
		e->op->priority = -1;

	if ((ret = expr_term(f, e->valr, top, scope)) > 0)
		return 1;
	e->sum_type = expr_get_sum_type(e->vall, e->valr);
	if (ret == -1)
		return -1;
	return 0;
err_valr_not_found:
	printf("amc: expr_binary: Value right not found!\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_check_end(char *endc, struct file *f, str *token, int top)
{
	if (token->len <= 0)
		return 2;
	if (top) {
		if (f->src[f->pos] == '\n'
				|| f->src[f->pos] == ';'
				|| f->src[f->pos] == ','
				|| f->src[f->pos] == ']')
			return 1;
		return 0;
	}
	if (f->src[f->pos] == ')' || *endc == ')')
		return 1;
	return 0;
}

enum YZ_TYPE *expr_get_sum_type(yz_val *l, yz_val *r)
{
	enum YZ_TYPE *sum_type_left = &l->type,
	             *sum_type_right = &r->type,
	             sign_left_type = l->type,
	             sign_right_type = r->type;
	if (YZ_IS_UNSIGNED_DIGIT(l->type) && YZ_IS_UNSIGNED_DIGIT(r->type))
		return MAX(&l->type, &r->type);
	if (l->type == AMC_EXPR) {
		sum_type_left = ((struct expr*)l->v)->sum_type;
		sign_left_type = *sum_type_left;
	}
	if (r->type == AMC_EXPR) {
		sum_type_right = ((struct expr*)r->v)->sum_type;
		sign_right_type = *sum_type_right;
	}
	if (YZ_IS_UNSIGNED_DIGIT(sign_left_type))
		sign_left_type -= 4;
	if (YZ_IS_UNSIGNED_DIGIT(sign_right_type))
		sign_right_type -= 4;
	return sign_left_type > sign_right_type
		? sum_type_left : sum_type_right;
}

int expr_operator(struct file *f, struct expr_operator **op, int top)
{
	int end = 0;
	str *tmp = str_new(),
	    token = TOKEN_NEW;
	if ((end = expr_read_token(f, &token, top)) == 2)
		goto err_eoe;
	str_append(tmp, token.len, token.s);
	str_append(tmp, 1, "\0");
	for (int i = 0; i < LENGTH(operators); i++) {
		if (strcmp(tmp->s, operators[i].sym) == 0) {
			*op = malloc(sizeof(**op));
			memcpy(*op, &operators[i], sizeof(**op));
			str_free(tmp);
			return end == 1 ? -1 : 0;
		}
	}
	str_free(tmp);
	return 1;
err_eoe:
	str_free(tmp);
	printf("amc: expr_operator: End of expression!\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_read_token(struct file *f, str *token, int top)
{
	int end = expr_check_end(token_read_before(TERM_END, token, f),
					f, token, top);
	if (end == 2)
		goto err_empty_token;
	if (top && f->src[f->pos] == '\n')
		return end;
	if (f->src[f->pos] == ']' || f->src[f->pos] == ',')
		return end;
	file_pos_next(f);
	file_skip_space(f);
	return end;
err_empty_token:
	printf("amc: expr_read_token: Empty token\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 2;
}

int expr_sub(struct file *f, struct expr **e, int top, struct scope *scope)
{
	struct expr *expr = calloc(1, sizeof(*expr));
	int end = 0;
	expr->vall = calloc(1, sizeof(*expr->vall));
	expr->valr = calloc(1, sizeof(*expr->valr));
	if ((end = expr_operator(f, &expr->op, top)) > 0)
		return 1;
	if (end == -1)
		goto err_valr_not_found;
	if ((end = expr_term(f, expr->valr, top, scope)) > 0)
		return 1;
	if (expr->op->priority < (*e)->op->priority) {
		// 1 + 2 * 3
		expr->vall->type = (*e)->valr->type;
		expr->vall->v = (*e)->valr->v;
		expr->sum_type = expr_get_sum_type(
				expr->vall,
				expr->valr);
		(*e)->valr->type = AMC_EXPR;
		(*e)->valr->v = expr;
		(*e)->sum_type = expr_get_sum_type(
				(*e)->vall,
				(*e)->valr);
	} else {
		// 1 * 2 + 3
		expr->vall->type = AMC_EXPR;
		expr->vall->v = *e;
		expr->sum_type = expr_get_sum_type(
				expr->vall,
				expr->valr);
		*e = expr;
	}
	if (end == 0) {
		if (top && (f->src[f->pos] == '\n' || f->src[f->pos] == ';'))
			return -1;
	}
	return end;
err_valr_not_found:
	printf("amc: expr_sub: Value right not found!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term(struct file *f, yz_val *v, int top, struct scope *scope)
{
	if (REGION_INT(f->src[f->pos], '0', '9')) {
		return expr_term_int(f, v, top);
	} else if (f->src[f->pos] == '\'') {
		return expr_term_chr(f, v, top);
	} else if (f->src[f->pos] == '(') {
		return expr_term_expr(f, v, top, scope);
	} else if (f->src[f->pos] == '[') {
		return expr_term_func(f, v, top, scope);
	}
	return 1;
}

int expr_term_chr(struct file *f, yz_val *v, int top)
{
	int end = 0;
	str token = TOKEN_NEW;
	if ((end = expr_read_token(f, &token, top)) == 2)
		goto err_eoe;
	token.s = &token.s[1];
	token.len -= 2;
	if (token.s[token.len] != '\'')
		goto err_char_not_end;
	v->type = YZ_CHAR;
	token.s = &token.s[1];
	token.len -= 2;
	return end == 1 ? -1 : 0;
err_eoe:
	printf("amc: expr_term_chr: End of expression!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_char_not_end:
	printf("amc: expr_term_chr: Character not end!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term_expr(struct file *f, yz_val *v, int top, struct scope *scope)
{
	file_pos_next(f);
	file_skip_space(f);
	v->type = AMC_EXPR;
	if ((v->v = parse_expr(f, 0, scope)) == NULL)
		goto err_cannot_parse_expr;
	if (top) {
		if (f->src[f->pos] != '\n'
				&& f->src[f->pos] != ';'
				&& f->src[f->pos] != ']'
				&& f->src[f->pos] != ',')
			return 0;
		return -1;
	}
	if (f->src[f->pos] == ')') {
		file_pos_next(f);
		file_skip_space(f);
		return -1;
	}
	return 0;
err_cannot_parse_expr:
	printf("amc: expr_term_expr: Cannot parse expression!\n"
			"| In l:%lld,c:%lld\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term_func(struct file *f, yz_val *v, int top, struct scope *scope)
{
	struct symbol *callee = NULL;
	int end = 0, ret = 0;
	char *err_msg;
	str token = TOKEN_NEW;
	if ((end = expr_read_token(f, &token, top)) == 2)
		return 1;
	token.s = &token.s[1];
	token.len -= 1;
	if (!symbol_find_in_group_in_scope(&token, &callee, scope, SYMG_FUNC))
		goto err_func_not_found;
	if (!callee->flags.in_block)
		goto err_func_not_in_block;
	v->type = AMC_SYM;
	v->v = callee;
	if ((ret = callee->parse_function(f, callee, scope)) == 0)
		return -1;
	if (ret > 0)
		return 1;
	return end == 1 ? -1 : 0;
err_func_not_found:
	err_msg = err_msg_get(token.s, token.len);
	printf("amc: expr_term_func: Function not found!\n"
			"| Token: \"%s\"\n"
			"|         ^\n"
			"| In l:%lld,c:%lld\n",
			err_msg,
			f->cur_line,
			f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_func_not_in_block:
	err_msg = err_msg_get(token.s, token.len);
	printf("amc: expr_term_func: Function cannot be called in block!\n"
			"| Token: \"%s\"\n"
			"|         ^\n"
			"| In l:%lld,c:%lld\n",
			err_msg,
			f->cur_line,
			f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term_int(struct file *f, yz_val *v, int top)
{
	int end = 0;
	char *err_msg;
	str token = TOKEN_NEW;
	if ((end = expr_read_token(f, &token, top)) == 2)
		goto err_eoe;
	if (str2int(&token, &v->l))
		goto err_not_num;
	v->type = yz_get_int_size(v->l);
	return end == 1 ? -1 : 0;
err_eoe:
	printf("amc: expr_term_int: End of expression!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_not_num:
	err_msg = err_msg_get(token.s, token.len);
	printf("amc: expr_term_int: Token: \"%s\" is not number!\n", err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_apply(struct expr *e)
{
	if (e->op == NULL && e->valr == NULL) {
		if (e->vall->type == AMC_EXPR)
			return expr_apply(e->vall->v);
		free_safe(e->op);
		free_safe(e->valr);
		return -1;
	}
	if (e->vall->type == AMC_EXPR) {
		if (expr_apply(e->vall->v))
			return 1;
	}
	if (e->valr->type == AMC_EXPR) {
		if (expr_apply(e->valr->v))
			return 1;
	}
	if (backend_call(ops[e->op->id])(e))
		goto err_backend_call;
	return 0;
err_backend_call:
	printf("amc: expr_apply: Backend call faulted!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

void expr_free(struct expr *e)
{
	if (e->vall != NULL) {
		if (e->vall->type == AMC_EXPR)
			expr_free(e->vall->v);
		free(e->vall);
	}
	free_safe(e->op);
	if (e->valr != NULL) {
		if (e->valr->type == AMC_EXPR)
			expr_free(e->valr->v);
		free(e->valr);
	}
	free(e);
}

struct expr *parse_expr(struct file *f, int top, struct scope *scope)
{
	struct expr *expr = calloc(1, sizeof(*expr));
	int ret = 0;
	expr->vall = calloc(1, sizeof(*expr->vall));
	expr->valr = calloc(1, sizeof(*expr->valr));
	if ((ret = expr_binary(f, expr, top, scope)) > 0)
		goto err_free_expr;
	if (ret == -1)
		return expr;
	while ((ret = expr_sub(f, &expr, top, scope)) != -1) {
		if (ret > 0)
			goto err_free_expr;
	}
	return expr;
err_free_expr:
	free_safe(expr);
	return NULL;
}
