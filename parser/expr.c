/**
 * TODO: 1. unary operator support.
 *       2. special operator support
 */
#include "expr.h"
#include "../include/backend.h"
#include "../include/identifier.h"
#include "../include/token.h"
#include "../utils/converter.h"
#include "../utils/die.h"
#include "../utils/utils.h"
#include <string.h>

static const char *TERM_END = ";) \t\n";

static int expr_binary(struct file *f, struct expr *e, int top);
static int expr_check_end(char *endc, struct file *f, str *token, int top);
static int expr_operator(struct file *f, struct expr_operator **op, int top);
static int expr_sub(struct file *f, struct expr **e, int top);
static int expr_term(struct file *f, yz_val *v, int top);
static int expr_term_chr(struct file *f, yz_val *v, int top);
static int expr_term_expr(struct file *f, yz_val *v);
static int expr_term_func(struct file *f, yz_val *v, int top);
static int expr_term_int(struct file *f, yz_val *v, int top);
static int expr_unary(str *token, struct expr *e);

int expr_binary(struct file *f, struct expr *e, int top)
{
	int ret = 0;
	if ((ret = expr_term(f, e->vall, top)) > 0)
		return 1;
	if (ret == -1) {
		free_safe(e->valr);
		return -1;
	}

	if ((ret = expr_operator(f, &e->op, top)) > 0)
		return 1;
	if (ret == -1)
		goto err_valr_not_found;
	if (!top)
		e->op->priority = -1;

	if ((ret = expr_term(f, e->valr, top)) > 0)
		return 1;
	if (ret == -1)
		return -1;
	return 0;
err_valr_not_found:
	printf("amc: expr_binary: Value right not found!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_check_end(char *endc, struct file *f, str *token, int top)
{
	if (token->len <= 0)
		return 2;
	if (top) {
		if (f->src[f->pos] == '\n')
			return 1;
		if (f->src[f->pos] == ';')
			return 1;
		return 0;
	}
	if (f->src[f->pos] == ')' || *endc == ')')
		return 1;
	return 0;
}

int expr_operator(struct file *f, struct expr_operator **op, int top)
{
	int end = 0;
	str *tmp = str_new(),
	    token = TOKEN_NEW;
	if ((end = expr_check_end(token_read_before(TERM_END, &token, f),
					f, &token, top)) == 2)
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
	printf("amc: expr_operator: End of expression!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

int expr_unary(str *token, struct expr *e)
{
	return 1;
}

int expr_sub(struct file *f, struct expr **e, int top)
{
	struct expr *expr = calloc(1, sizeof(*expr));
	int end = 0;
	expr->vall = calloc(1, sizeof(*expr->vall));
	expr->valr = calloc(1, sizeof(*expr->valr));
	if ((end = expr_operator(f, &expr->op, top)) > 0)
		return 1;
	if (end == -1)
		goto err_valr_not_found;
	if ((end = expr_term(f, expr->valr, top)) > 0)
		return 1;
	if (expr->op->priority < (*e)->op->priority) {
		// 1 + 2 * 3
		expr->vall->type = (*e)->valr->type;
		expr->vall->v = (*e)->valr->v;
		(*e)->valr->type = AMC_SUB_EXPR;
		(*e)->valr->v = expr;
	} else {
		// 1 * 2 + 3
		expr->vall->type = AMC_SUB_EXPR;
		expr->vall->v = *e;
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

int expr_term(struct file *f, yz_val *v, int top)
{
	struct symbol *sym = NULL;
	if (REGION_INT(f->src[f->pos], '0', '9')) {
		return expr_term_int(f, v, top);
	} else if (f->src[f->pos] == '\'') {
		return expr_term_chr(f, v, top);
	} else if (f->src[f->pos] == '(') {
		return expr_term_expr(f, v);
	} else if (f->src[f->pos] == '[') {
		return expr_term_func(f, v, top);
	}
	return 1;
}

int expr_term_chr(struct file *f, yz_val *v, int top)
{
	int end = 0;
	str token = TOKEN_NEW;
	if ((end = expr_check_end(token_read_before(TERM_END, &token, f),
					f, &token, top)) == 2)
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

int expr_term_expr(struct file *f, yz_val *v)
{
	file_pos_next(f);
	file_skip_space(f);
	v->type = AMC_SUB_EXPR;
	v->v = parse_expr(f, 0);
	if (f->src[f->pos] == ')') {
		file_pos_next(f);
		file_skip_space(f);
		return -1;
	}
	return 0;
}

int expr_term_func(struct file *f, yz_val *v, int top)
{
	struct symbol *callee = NULL;
	int end = 0;
	char *err_msg;
	str token = TOKEN_NEW;
	if ((end = expr_check_end(token_read_before(TERM_END, &token, f),
					f, &token, top)) == 2)
		goto err_eoe;
	token.s = &token.s[1];
	token.len -= 1;
	if (!symbol_find_in_group(&token, SYMG_FUNC, &callee))
		goto err_func_not_found;
	if (!callee->flags.in_block)
		goto err_func_not_in_block;
	v->type = AMC_SYM;
	v->v = callee;
	return end == 1 ? -1 : 0;
err_eoe:
	printf("amc: expr_term_func: End of expression!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
err_func_not_found:
	err_msg = err_msg_get(token.s, token.len);
	printf("amc: expr_term_func: Function not found!\n"
			"| Token: \"%s\"\n"
			"|         ^\n"
			"| In: l:%lld,c:%lld\n",
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
			"| In: l:%lld,c:%lld\n",
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
	if ((end = expr_check_end(token_read_before(TERM_END, &token, f),
					f, &token, top)) == 2)
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

int parse_assignment_expr(yz_val *l, yz_val *r)
{
	struct symbol *sym = l->v;
	struct symbol *tmp = NULL;
	str token = {.s = (char*)sym->name, .len = sym->name_len};
	if (r->type == AMC_SYM)
		return 1;
	if (l->type != AMC_SYM || l->v == NULL)
		return 1;
	if (symbol_find_in_group(&token, 1, &tmp)) {
		if (!sym->flags.mut)
			goto err_cannot_reassign_immut;
		backend_call(var_set)(&token, r);
		return 0;
	}
	if (sym->flags.mut) {
		backend_call(var_set)(&token, r);
		sym->parse_function = parse_mut_var;
	} else {
		backend_call(var_immut_set)(&token, r);
		sym->parse_function = parse_immut_var;
	}
	symbol_register(sym, 1);
	return 0;
err_cannot_reassign_immut:
	printf("amc: cannot reassign immutable variable.\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return 1;
}

yz_val *expr_apply(struct expr *e)
{
	yz_val *result = NULL,
	       *tmp_v  = NULL,
	       v = {.l = 0, .type = AMC_ERR_TYPE};
	if (e->op == NULL && e->valr == NULL) {
		if (e->vall == NULL)
			goto err_incomplete_expr;
		return e->vall;
	}
	if (e->vall->type == AMC_SUB_EXPR) {
		tmp_v = expr_apply(e->vall->v);
		v.type = tmp_v->type;
		v.l = tmp_v->l;
		result = tmp_v;
	} else {
		result = e->vall;
	}
	if (e->valr->type == AMC_SUB_EXPR) {
		tmp_v = expr_apply(e->valr->v);
		if (v.type != tmp_v->type)
			goto err_wrong_type;
	} else if (result == NULL) {
		result = e->valr;
	}
	if (backend_call(ops[e->op->id])(e->vall, e->valr))
		goto err_backend_call;
	return result;
err_incomplete_expr:
	printf("amc: expr_apply: Expression is incomplete!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
err_wrong_type:
	printf("amc: expr_apply: Wrong type!\n"
			"| Left value type:  \"%d\"\n"
			"| Right value type: \"%d\"\n",
			e->vall->type,
			e->valr->type);
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
err_backend_call:
	printf("amc: expr_apply: Backend call faulted!\n");
	backend_stop(BE_STOP_SIGNAL_ERR);
	return NULL;
}

void expr_free(struct expr *e)
{
	if (e->vall != NULL) {
		if (e->vall->type == AMC_SUB_EXPR)
			expr_free(e->vall->v);
		free(e->vall);
	}
	free_safe(e->op);
	if (e->valr != NULL) {
		if (e->valr->type == AMC_SUB_EXPR)
			expr_free(e->valr->v);
		free(e->valr);
	}
	free(e);
}

struct expr *parse_expr(struct file *f, int top)
{
	struct expr *expr = calloc(1, sizeof(*expr));
	int ret = 0;
	expr->vall = calloc(1, sizeof(*expr->vall));
	expr->valr = calloc(1, sizeof(*expr->valr));
	if ((ret = expr_binary(f, expr, top)) > 0)
		goto err_free_expr;
	if (ret == -1)
		return expr;
	while ((ret = expr_sub(f, &expr, top)) != -1) {
		if (ret > 0)
			goto err_free_expr;
	}
	return expr;
err_free_expr:
	free_safe(expr);
	return NULL;
}
