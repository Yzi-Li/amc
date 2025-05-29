#include "include/expr.h"
#include "include/identifier.h"
#include "include/keywords.h"
#include "include/op.h"
#include "../include/backend.h"
#include "../include/token.h"
#include "../utils/converter.h"
#include "../utils/utils.h"
#include <string.h>

#define EXPR_END -1
#define EXPR_OP_EMPTY -2
#define EXPR_TERM_END -1
#define EXPR_TOK_END 1
#define EXPR_TOK_END_DIRECT 3

static const char *TERM_END = ";),]}[ \t\n";

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

static int expr_binary(struct file *f, struct expr *e, int top,
		struct scope *scope);
static int expr_check_end(struct file *f);
static int expr_check_end_special(struct file *f, int top);
static int expr_check_end_top(struct file *f, str *tok);
static enum YZ_TYPE *expr_get_sum_type(yz_val *l, yz_val *r);
static int expr_operator(struct file *f, struct expr_operator **op, int top);
static int expr_read_token(struct file *f, str *token, int top);
static int expr_single_term(struct expr *e);
static int expr_sub(struct file *f, struct expr **e, int top,
		struct scope *scope);
static int expr_term(struct file *f, yz_val *v, int top, struct scope *scope);
static int expr_term_chr(struct file *f, yz_val *v, int top);
static int expr_term_expr(struct file *f, yz_val *v, int top,
		struct scope *scope);
static int expr_term_func(struct file *f, yz_val *v, int top,
		struct scope *scope);
static int expr_term_identifier(struct file *f, yz_val *v, int top,
		struct scope *scope);
static int expr_term_int(struct file *f, yz_val *v, int top);
static int expr_unary(struct file *f, yz_val *v, struct expr_operator *op,
		int top, struct scope *scope);
static struct expr_operator *expr_unary_get_op(char c);

int expr_binary(struct file *f, struct expr *e, int top, struct scope *scope)
{
	int ret = 0;
	if ((ret = expr_term(f, e->vall, top, scope)) > 0)
		return 1;
	if (ret == EXPR_TERM_END)
		return expr_single_term(e);

	if ((ret = expr_operator(f, &e->op, top)) > 0)
		return 1;
	if (ret == EXPR_TERM_END)
		goto err_valr_not_found;
	if (ret == EXPR_OP_EMPTY)
		return EXPR_TERM_END;
	if (!top)
		e->op->priority = -1;

	if ((ret = expr_term(f, e->valr, top, scope)) > 0)
		return 1;
	if ((e->sum_type = expr_get_sum_type(e->vall, e->valr)) == NULL)
		return 1;
	if (ret == EXPR_TERM_END)
		return EXPR_TERM_END;
	return 0;
err_valr_not_found:
	printf("amc: expr_binary: %lld,%lld: Value right not found!\n",
			f->cur_line, f->cur_column);
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

enum YZ_TYPE *expr_get_sum_type(yz_val *l, yz_val *r)
{
	yz_val *val = NULL;
	if ((val = yz_type_max(l, r)) == NULL)
		return NULL;
	return yz_get_raw_type(val);
}

int expr_operator(struct file *f, struct expr_operator **op, int top)
{
	int end = 0;
	str *tmp = str_new(),
	    token = TOKEN_NEW;
	if ((end = expr_read_token(f, &token, top)) == 2)
		goto err_eoe;
	if (end == EXPR_TOK_END_DIRECT)
		return EXPR_OP_EMPTY;
	str_append(tmp, token.len, token.s);
	str_append(tmp, 1, "\0");
	for (int i = 0; i < LENGTH(operators); i++) {
		if (strcmp(tmp->s, operators[i].sym) == 0) {
			*op = malloc(sizeof(**op));
			memcpy(*op, &operators[i], sizeof(**op));
			str_free(tmp);
			return end == 1 ? EXPR_TERM_END : 0;
		}
	}
	str_free(tmp);
	return 1;
err_eoe:
	str_free(tmp);
	printf("|< amc: expr_operator: End of expression!\n");
	return 1;
}

int expr_read_token(struct file *f, str *token, int top)
{
	int ret = 0;
	token_read_before(TERM_END, token, f);
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
	if (e->vall->type == AMC_EXPR) {
		e->sum_type = ((struct expr*)e->vall->v)->sum_type;
	} else if (e->vall->type == AMC_SYM) {
		e->sum_type = &((struct symbol*)e->vall->v)->result_type.type;
	} else {
		e->sum_type = &e->vall->type;
	}
	free_safe(e->valr);
	free_safe(e->op);
	return EXPR_END;
}

int expr_sub(struct file *f, struct expr **e, int top, struct scope *scope)
{
	struct expr *expr = calloc(1, sizeof(*expr));
	int end = 0;
	expr->vall = calloc(1, sizeof(*expr->vall));
	expr->valr = calloc(1, sizeof(*expr->valr));
	if ((end = expr_operator(f, &expr->op, top)) > 0)
		return 1;
	if (end == EXPR_TERM_END)
		goto err_valr_not_found;
	if (end == EXPR_OP_EMPTY)
		return EXPR_END;
	if ((end = expr_term(f, expr->valr, top, scope)) > 0)
		return 1;
	if (expr->op->priority < (*e)->op->priority) {
		// 1 + 2 * 3
		expr->vall->type = (*e)->valr->type;
		expr->vall->v = (*e)->valr->v;
		if ((expr->sum_type = expr_get_sum_type(
				expr->vall,
				expr->valr)) == NULL)
			return 1;
		(*e)->valr->type = AMC_EXPR;
		(*e)->valr->v = expr;
		if (((*e)->sum_type = expr_get_sum_type(
				(*e)->vall,
				(*e)->valr)) == NULL)
			return 1;
	} else {
		// 1 * 2 + 3
		expr->vall->type = AMC_EXPR;
		expr->vall->v = *e;
		if ((expr->sum_type = expr_get_sum_type(
				expr->vall,
				expr->valr)) == NULL)
			return 1;
		*e = expr;
	}
	if (end == 0) {
		if (top && (f->src[f->pos] == '\n' || f->src[f->pos] == ';'))
			return EXPR_END;
	}
	return end;
err_valr_not_found:
	printf("amc: expr_sub: Value right not found!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term(struct file *f, yz_val *v, int top, struct scope *scope)
{
	struct expr_operator *unary = NULL;
	if (CHR_IS_NUM(f->src[f->pos])) {
		return expr_term_int(f, v, top);
	} else if (f->src[f->pos] == '\'') {
		return expr_term_chr(f, v, top);
	} else if (f->src[f->pos] == '(') {
		return expr_term_expr(f, v, top, scope);
	} else if (f->src[f->pos] == '[') {
		return expr_term_func(f, v, top, scope);
	} else if ((unary = expr_unary_get_op(f->src[f->pos])) != NULL) {
		return expr_unary(f, v, unary, top, scope);
	}
	return expr_term_identifier(f, v, top, scope);
}

int expr_term_chr(struct file *f, yz_val *v, int top)
{
	int end = 0;
	str token = TOKEN_NEW;
	if ((end = expr_read_token(f, &token, top)) == 2)
		goto err_eoe;
	if (end == EXPR_TOK_END_DIRECT)
		return EXPR_TERM_END;
	token.s = &token.s[1];
	token.len -= 2;
	if (token.s[token.len] != '\'')
		goto err_char_not_end;
	v->type = YZ_CHAR;
	token.s = &token.s[1];
	token.len -= 2;
	return end == 1 ? EXPR_TERM_END : 0;
err_eoe:
	printf("|< amc: expr_term_chr: End of expression!\n");
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
	if (f->src[f->pos] != ')')
		return 1;
	file_pos_next(f);
	file_skip_space(f);
	if (expr_check_end_special(f, top))
		return EXPR_TERM_END;
	return 0;
err_cannot_parse_expr:
	printf("amc: expr_term_expr: %lld,%lld: Cannot parse expression!\n",
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
	file_pos_next(f);
	file_skip_space(f);
	if ((end = expr_read_token(f, &token, top)) == 2)
		return 1;
	if (end == EXPR_TOK_END_DIRECT)
		return EXPR_TERM_END;
	if (!symbol_find_in_group_in_scope(&token, &callee, scope, SYMG_FUNC))
		goto err_func_not_found;
	if (!callee->flags.in_block)
		goto err_func_not_in_block;
	v->type = AMC_SYM;
	v->v = callee;
	if ((ret = callee->parse_function(f, callee, scope)) > 0)
		return 1;
	if (expr_check_end_special(f, top))
		return EXPR_TERM_END;
	return 0;
err_func_not_found:
	err_msg = str2chr(token.s, token.len);
	printf("amc: expr_term_func: %lld,%lld: Function not found!\n"
			"| Token: \"%s\"\n"
			"|         ^\n",
			f->cur_line, f->cur_column,
			err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_func_not_in_block:
	err_msg = str2chr(token.s, token.len);
	printf("amc: expr_term_func: %lld,%lld: "
			"Function cannot be called in block!\n"
			"| Token: \"%s\"\n"
			"|         ^\n",
			f->cur_line, f->cur_column,
			err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_term_identifier(struct file *f, yz_val *v, int top,
		struct scope *scope)
{
	int end = 0;
	str token = TOKEN_NEW;
	if ((end = expr_read_token(f, &token, top)) == 2)
		goto err_eoe;
	if (end == EXPR_TOK_END_DIRECT)
		return EXPR_TERM_END;
	if (identifier_read(f, &token, v, scope) > 1)
		return 1;
	if (expr_check_end_special(f, top))
		return EXPR_TERM_END;
	return 0;
err_eoe:
	printf("|< amc: expr_term_identifier: End of expression!\n");
	return 1;
}

int expr_term_int(struct file *f, yz_val *v, int top)
{
	int end = 0;
	char *err_msg;
	str token = TOKEN_NEW;
	if ((end = expr_read_token(f, &token, top)) == 2)
		goto err_eoe;
	if (end == EXPR_TOK_END_DIRECT)
		return EXPR_TERM_END;
	if (str2int(&token, &v->l))
		goto err_not_num;
	v->type = yz_get_int_size(v->l);
	return end == 1 ? EXPR_TERM_END : 0;
err_eoe:
	printf("|< amc: expr_term_int: End of expression!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_not_num:
	err_msg = str2chr(token.s, token.len);
	printf("amc: expr_term_int: Token: \"%s\" is not number!\n", err_msg);
	free(err_msg);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_unary(struct file *f, yz_val *v, struct expr_operator *op, int top,
		struct scope *scope)
{
	int ret = 0;
	struct expr *unary = NULL;
	file_pos_next(f);
	if (!file_try_skip_space(f))
		goto err_eou;
	v->type = AMC_EXPR;
	unary = malloc(sizeof(*unary));
	unary->vall = NULL;
	unary->valr = malloc(sizeof(*unary->valr));
	unary->op = op;
	v->v = unary;
	if ((ret = expr_term(f, unary->valr, top, scope)) > 0)
		return 1;
	unary->sum_type = yz_get_raw_type(unary->valr);
	return ret;
err_eou:
	printf("amc: expr_unary: %lld,%lld: Unary expression is empty!\n",
			f->cur_line, f->cur_column);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

struct expr_operator *expr_unary_get_op(char c)
{
	struct expr_operator *result = NULL;
	for (int i = 0; i < LENGTH(unary_ops); i++) {
		if (c != unary_ops[i].sym[0])
			continue;
		result = malloc(sizeof(*result));
		memcpy(result, &unary_ops[i], sizeof(*result));
		return result;
	}
	return NULL;
}

int expr_apply(struct expr *e, struct scope *scope)
{
	if (EXPR_IS_SINGLE_TERM(e)) {
		if (e->vall->type == AMC_EXPR)
			return expr_apply(e->vall->v, scope);
		return -1;
	}
	if (EXPR_IS_UNARY(e))
		return op_apply_special(e, scope);
	if (e->vall->type == AMC_EXPR) {
		if (expr_apply(e->vall->v, scope) > 0)
			return 1;
	}
	if (e->valr->type == AMC_EXPR) {
		if (expr_apply(e->valr->v, scope) > 0)
			return 1;
	}
	if (e->op->id >= OP_SPECIAL_START)
		return op_apply_special(e, scope);
	if (backend_call(ops[e->op->id])(e))
		goto err_backend_call;
	return 0;
err_backend_call:
	printf("amc: expr_apply: Backend call failed!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

void expr_free(struct expr *e)
{
	expr_free_val(e->vall);
	expr_free_val(e->valr);
	free_safe(e->op);
	free(e);
}

void expr_free_val(yz_val *v)
{
	if (v == NULL)
		return;
	if (v->type == AMC_EXPR)
		expr_free(v->v);
	free(v);
}

struct expr *parse_expr(struct file *f, int top, struct scope *scope)
{
	struct expr *expr = calloc(1, sizeof(*expr));
	int ret = 0;
	expr->vall = calloc(1, sizeof(*expr->vall));
	expr->valr = calloc(1, sizeof(*expr->valr));
	if ((ret = expr_binary(f, expr, top, scope)) > 0)
		goto err_free_expr;
	if (ret == EXPR_END)
		return expr;
	while ((ret = expr_sub(f, &expr, top, scope)) != EXPR_END) {
		if (ret > 0)
			goto err_free_expr;
	}
	return expr;
err_free_expr:
	free_safe(expr);
	return NULL;
}
