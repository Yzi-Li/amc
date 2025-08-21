/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/expr.h"
#include "include/func.h"
#include "include/identifier.h"
#include "include/keywords.h"
#include "include/null.h"
#include "include/op.h"
#include "include/token.h"
#include "../include/array.h"
#include "../include/backend.h"
#include "../include/const.h"
#include "../include/parser.h"
#include "../include/token.h"
#include "../utils/converter.h"
#include "../utils/utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define EXPR_END -1
#define EXPR_OP_EMPTY -2
#define EXPR_TERM_END -1
#define EXPR_TOK_END 1
#define EXPR_TOK_END_DIRECT 3

static const struct expr_operator operators[] = {
	{"*",   1, OP_MUL},
	{"/",   1, OP_DIV},
	{"+",   2, OP_ADD},
	{"-",   2, OP_SUB},

	{"==",  3, OP_EQ},
	{"!=",  3, OP_NE},
	{"<",   3, OP_LT},
	{"<=",  3, OP_LE},
	{">",   3, OP_GT},
	{">=",  3, OP_GE},

	{"and", 4, OP_AND},
	{"or",  4, OP_OR },

	{"=",   5, OP_ASSIGN    },
	{"+=",  5, OP_ASSIGN_ADD},
	{"/=",  5, OP_ASSIGN_DIV},
	{"*=",  5, OP_ASSIGN_MUL},
	{"-=",  5, OP_ASSIGN_SUB}
};

static const struct expr_operator unary_ops[] = {
	{"*", 0, OP_EXTRACT_VAL},
	{"&", 0, OP_GET_ADDR   }
};

static int expr_assign(struct parser *parser, struct expr *e);
static int expr_binary(struct parser *parser, int top, struct expr *e);
static int expr_binary_read_op(struct parser *parser, int top, struct expr *e);
static int expr_check_end(struct file *f);
static int expr_check_end_special(struct file *f, int top);
static int expr_check_end_top(struct file *f, str *tok);
static yz_type *expr_get_sum_type(yz_type *l, yz_type *r);
static int expr_operator(struct file *f, int top, struct expr *e);
static int expr_read_token(struct file *f, int top, str *token);
static int expr_single_term(struct expr *e);
static int expr_sub(struct parser *parser, int top, struct expr **e);
static int expr_sub_merge_prev(struct expr *prev, struct expr *cur);
static int expr_sub_append(struct expr **prev, struct expr *cur);
static int expr_term(struct parser *parser, int top, yz_val *v);
static int expr_term_chr(struct parser *parser, int top, yz_val *v);
static int expr_term_expr(struct parser *parser, int top, yz_val *v);
static int expr_term_func(struct parser *parser, int top, yz_val *v);
static int expr_term_identifier(struct parser *parser, int top, yz_val *v);
static int expr_term_int(struct parser *parser, int top, yz_val *v);
static int expr_term_null(struct parser *parser, int top, yz_val *v);
static int expr_term_str(struct parser *parser, int top, yz_val *v);
static int expr_unary(struct parser *parser, int top, yz_val *v,
		enum OP_ID op);
static enum OP_ID expr_unary_get_op(char c);

int expr_assign(struct parser *parser, struct expr *e)
{
	if (op_assign(parser, e))
		return 1;
	return EXPR_END;
}

int expr_binary(struct parser *parser, int top, struct expr *e)
{
	int ret = 0;
	if ((ret = expr_term(parser, top, e->vall)) > 0)
		return 1;
	if (ret == EXPR_TERM_END)
		return expr_single_term(e);
	if ((ret = expr_binary_read_op(parser, top, e)) != 0)
		return ret;
	if ((ret = expr_term(parser, top, e->valr)) > 0)
		return 1;
	if (OP_IS_CMP(e->op)) {
		e->sum_type = &e->vall->type;
	} else {
		if ((e->sum_type = expr_get_sum_type(&e->vall->type,
						&e->valr->type))
				== NULL)
			return 1;
	}
	if (ret == EXPR_TERM_END)
		return EXPR_TERM_END;
	return 0;
}

int expr_binary_read_op(struct parser *parser, int top, struct expr *e)
{
	int ret = 0;
	if ((ret = expr_operator(parser->f, top, e)) > 0)
		return 1;
	if (ret == EXPR_TERM_END)
		goto err_valr_not_found;
	if (ret == EXPR_OP_EMPTY)
		return EXPR_TERM_END;
	if (!top)
		e->priority = -1;
	if (REGION_INT(e->op, OP_ASSIGN, OP_ASSIGN_SUB))
		return expr_assign(parser, e);
	return 0;
err_valr_not_found:
	printf("amc: expr_binary_read_op: %lld,%lld: Value right not found!\n",
			parser->f->cur_line, parser->f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_check_end(struct file *f)
{
	if (f->src[f->pos] == ','
			|| f->src[f->pos] == ')'
			|| f->src[f->pos] == ']'
			|| f->src[f->pos] == '}')
		return EXPR_TOK_END;
	return 0;
}

int expr_check_end_special(struct file *f, int top)
{
	if (top)
		return expr_check_end_top(f, NULL);
	return expr_check_end(f);
}

int expr_check_end_top(struct file *f, str *token)
{
	if (token != NULL && token->s[0] == '=' && token->s[1] == '>')
		return EXPR_TOK_END_DIRECT;
	if (f->src[f->pos] == '\n'
			|| parse_comment(f)
			|| expr_check_end(f))
		return EXPR_TOK_END;
	return 0;
}

yz_type *expr_get_sum_type(yz_type *l, yz_type *r)
{
	yz_type *type = NULL;
	if ((type = yz_type_max(l, r)) == NULL)
		goto err_get_failed;
	return type;
err_get_failed:
	printf("amc: expr_get_sum_type: Get expression's sum type failed!\n");
	return NULL;
}

int expr_operator(struct file *f, int top, struct expr *e)
{
	int end = 0;
	str *tmp = str_new(),
	    token = TOKEN_NEW;
	if ((end = expr_read_token(f, top, &token)) == 2)
		goto err_eoe;
	if (end == EXPR_TOK_END_DIRECT)
		return EXPR_OP_EMPTY;
	str_append(tmp, token.len, token.s);
	str_append(tmp, 1, "\0");
	for (int i = 0; i < LENGTH(operators); i++) {
		if (strcmp(tmp->s, operators[i].sym) != 0)
			continue;
		e->op = operators[i].id;
		e->priority = operators[i].priority;
		str_free(tmp);
		return end == 1 ? EXPR_TERM_END : 0;
	}
	str_free(tmp);
	return 1;
err_eoe:
	str_free(tmp);
	printf("|< amc: expr_operator: End of expression!\n");
	return 1;
}

int expr_read_token(struct file *f, int top, str *token)
{
	int ret = 0;
	token_read_before(SPECIAL_TOKEN_END, token, f);
	if (token->len <= 0)
		goto err_empty_token;
	file_skip_space(f);
	if (top && ((ret = expr_check_end_top(f, token)) != 0))
		return ret;
	if (!expr_check_end(f))
		return 0;
	return EXPR_TOK_END;
err_empty_token:
	printf("amc: expr_read_token: %lld,%lld: Empty token\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 2;
}

int expr_single_term(struct expr *e)
{
	if (e->vall->type.type == AMC_EXPR) {
		e->sum_type = e->vall->expr->sum_type;
	} else if (e->vall->type.type == AMC_SYM) {
		e->sum_type = &e->vall->sym->result_type;
	} else {
		e->sum_type = &e->vall->type;
	}
	e->op = -1;
	free_safe(e->valr);
	return EXPR_END;
}

int expr_sub(struct parser *parser, int top, struct expr **e)
{
	struct expr *expr = calloc(1, sizeof(*expr));
	int end = 0;
	expr->vall = calloc(1, sizeof(*expr->vall));
	expr->valr = calloc(1, sizeof(*expr->valr));
	if ((end = expr_operator(parser->f, top, expr)) > 0)
		return 1;
	if (end == EXPR_TERM_END)
		goto err_valr_not_found;
	if (end == EXPR_OP_EMPTY)
		return EXPR_END;
	if ((end = expr_term(parser, top, expr->valr)) > 0)
		return 1;
	if (expr->priority < (*e)->priority) {
		// 1 + 2 * 3
		if (expr_sub_merge_prev(*e, expr))
			return 1;
	} else {
		// 1 * 2 + 3
		if (expr_sub_append(e, expr))
			return 1;
	}
	if (end == 0) {
		if (top && (parser->f->src[parser->f->pos] == '\n'
					|| parser->f->src[parser->f->pos]
					== ';'))
			return EXPR_END;
	}
	return end;
err_valr_not_found:
	printf("amc: expr_sub: Value right not found!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_sub_merge_prev(struct expr *prev, struct expr *cur)
{
	cur->vall->v = prev->valr->v;
	cur->vall->type = prev->valr->type;
	if ((cur->sum_type = expr_get_sum_type(
					&cur->vall->type,
					&cur->valr->type)) == NULL)
		return 1;
	prev->valr->v = cur;
	prev->valr->type.type = AMC_EXPR;
	prev->valr->type.v = prev->valr->v;
	if ((prev->sum_type = expr_get_sum_type(
					&prev->vall->type,
					&prev->valr->type)) == NULL)
		return 1;
	return 0;
}

int expr_sub_append(struct expr **prev, struct expr *cur)
{
	cur->vall->v = *prev;
	cur->vall->type.type = AMC_EXPR;
	cur->vall->type.v = cur->vall->v;
	if ((cur->sum_type = expr_get_sum_type(
					&cur->vall->type,
					&cur->valr->type)) == NULL)
		return 1;
	*prev = cur;
	return 0;
}

int expr_term(struct parser *parser, int top, yz_val *v)
{
	enum OP_ID unary;
	if (CHR_IS_NUM(parser->f->src[parser->f->pos])) {
		return expr_term_int(parser, top, v);
	} else if (parser->f->src[parser->f->pos] == '\'') {
		return expr_term_chr(parser, top, v);
	} else if (parser->f->src[parser->f->pos] == '(') {
		return expr_term_expr(parser, top, v);
	} else if (parser->f->src[parser->f->pos] == '[') {
		return expr_term_func(parser, top, v);
	} else if (parser->f->src[parser->f->pos] == '"') {
		return expr_term_str(parser, top, v);
	} else if ((unary = expr_unary_get_op(parser->f->src[parser->f->pos]))
			!= -1) {
		return expr_unary(parser, top, v, unary);
	} else if (parser->f->src[parser->f->pos] == CHR_NULL[0]
			&& CHR_IS_NULL(&parser->f->src[parser->f->pos])) {
		return expr_term_null(parser, top, v);
	}
	return expr_term_identifier(parser, top, v);
}

int expr_term_chr(struct parser *parser, int top, yz_val *v)
{
	int end = 0;
	str token = TOKEN_NEW;
	if ((end = expr_read_token(parser->f, top, &token)) == 2)
		goto err_eoe;
	if (end == EXPR_TOK_END_DIRECT)
		return EXPR_TERM_END;
	if (token.s[token.len - 1] != '\'')
		goto err_char_not_end;
	v->type.type = YZ_CHAR;
	v->i = token.s[1];
	return end == 1 ? EXPR_TERM_END : 0;
err_eoe:
	printf("amc: expr_term_chr: End of expression!\n");
	return 1;
err_char_not_end:
	printf("amc: expr_term_chr: Character not end!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term_expr(struct parser *parser, int top, yz_val *v)
{
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	if ((v->v = parse_expr(parser, 0)) == NULL)
		goto err_cannot_parse_expr;
	if (parser->f->src[parser->f->pos] != ')')
		return 1;
	v->type.type = AMC_EXPR;
	v->type.v = v->v;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	if (expr_check_end_special(parser->f, top))
		return EXPR_TERM_END;
	return 0;
err_cannot_parse_expr:
	printf("amc: expr_term_expr: %lld,%lld: Cannot parse expression!\n",
			parser->f->cur_line, parser->f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term_func(struct parser *parser, int top, yz_val *v)
{
	struct symbol *callee = NULL;
	if (func_call_read(parser, &callee))
		return 1;
	v->v = callee;
	v->type.type = AMC_SYM;
	v->type.v = v->v;
	if (expr_check_end_special(parser->f, top))
		return EXPR_TERM_END;
	return 0;
}

int expr_term_identifier(struct parser *parser, int top, yz_val *v)
{
	if (identifier_read(parser, v) > 1)
		return 1;
	if (expr_check_end_special(parser->f, top))
		return EXPR_TERM_END;
	return 0;
}

int expr_term_int(struct parser *parser, int top, yz_val *v)
{
	int end = 0;
	char *err_msg;
	str token = TOKEN_NEW;
	if ((end = expr_read_token(parser->f, top, &token)) == 2)
		goto err_eoe;
	if (end == EXPR_TOK_END_DIRECT)
		return EXPR_TERM_END;
	if (str2int(&token, &v->l))
		goto err_not_num;
	v->type.type = yz_get_int_size(v->l);
	return end == 1 ? EXPR_TERM_END : 0;
err_eoe:
	printf("amc: expr_term_int: End of expression!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_not_num:
	err_msg = str2chr(token.s, token.len);
	printf("amc: expr_term_int: Token: \"%s\" is not number!\n", err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term_null(struct parser *parser, int top, yz_val *v)
{
	file_pos_nnext(strlen(CHR_NULL), parser->f);
	file_skip_space(parser->f);
	v->type.type = YZ_NULL;
	if (expr_check_end_special(parser->f, top))
		return EXPR_TERM_END;
	return 0;
}

int expr_term_str(struct parser *parser, int top, yz_val *v)
{
	yz_const *c = NULL;
	yz_array_type *arr = NULL;
	str *s = NULL, token = TOKEN_NEW;
	file_pos_next(parser->f);
	if (token_read_before("\"", &token, parser->f) == NULL)
		return 1;
	s = str_new();
	if (str_copy(&token, s))
		return 1;
	file_pos_next(parser->f);
	file_skip_space(parser->f);
	c = calloc(1, sizeof(*c));
	if (backend_call(const_def_str)(&c->be_data, s))
		goto err_backend_failed;
	str_free(s);
	arr = calloc(1, sizeof(*arr));
	arr->len = token.len;
	arr->type.type = YZ_CHAR;
	c->val.type.type = YZ_ARRAY;
	c->val.type.v = arr;
	v->type.type = YZ_CONST;
	v->type.v = &c->val.type;
	v->v = c;
	if (expr_check_end_special(parser->f, top))
		return EXPR_TERM_END;
	return 0;
err_backend_failed:
	printf("amc: expr_term_str: Backend call failed!\n");
	return 1;
}

int expr_unary(struct parser *parser, int top, yz_val *v, enum OP_ID op)
{
	int ret = 0;
	struct expr *unary = NULL;
	file_pos_next(parser->f);
	if (!file_try_skip_space(parser->f))
		goto err_eou;
	unary = malloc(sizeof(*unary));
	unary->vall = NULL;
	unary->valr = calloc(1, sizeof(*unary->valr));
	unary->op = op;
	unary->priority = 0;
	v->v = unary;
	v->type.type = AMC_EXPR;
	v->type.v = v->v;
	if ((ret = expr_term(parser, top, unary->valr)) > 0)
		return 1;
	unary->sum_type = &unary->valr->type;
	return ret;
err_eou:
	printf("amc: expr_unary: %lld,%lld: Unary expression is empty!\n",
			parser->f->cur_line, parser->f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

enum OP_ID expr_unary_get_op(char c)
{
	for (int i = 0; i < LENGTH(unary_ops); i++) {
		if (c != unary_ops[i].sym[0])
			continue;
		return unary_ops[i].id;
	}
	return -1;
}

int expr_apply(struct parser *parser, struct expr *e)
{
	if (EXPR_IS_SINGLE_TERM(e)) {
		if (e->vall->type.type == AMC_EXPR)
			return expr_apply(parser, e->vall->v);
		return -1;
	}
	if (EXPR_IS_UNARY(e))
		return op_apply_special(parser, e);
	if (e->vall->type.type == AMC_EXPR) {
		if (expr_apply(parser, e->vall->v) > 0)
			return 1;
	}
	if (e->valr->type.type == AMC_EXPR) {
		if (expr_apply(parser, e->valr->v) > 0)
			return 1;
	}
	if (e->op >= OP_SPECIAL_START)
		return op_apply_special(parser, e);
	if (OP_IS_CMP(e->op))
		return op_apply_cmp(e);
	if (backend_call(ops[e->op])(e))
		goto err_backend_call;
	return 0;
err_backend_call:
	printf("amc: expr_apply: Backend call failed!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

struct expr *parse_expr(struct parser *parser, int top)
{
	struct expr *expr = calloc(1, sizeof(*expr));
	int ret = 0;
	expr->vall = calloc(1, sizeof(*expr->vall));
	expr->valr = calloc(1, sizeof(*expr->valr));
	if ((ret = expr_binary(parser, top, expr)) > 0)
		goto err_free_expr;
	if (ret == EXPR_END)
		return expr;
	while ((ret = expr_sub(parser, top, &expr)) != EXPR_END) {
		if (ret > 0)
			goto err_free_expr;
	}
	return expr;
err_free_expr:
	free_expr(expr);
	return NULL;
}
