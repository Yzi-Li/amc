#include "../include/token.h"
#include <stdlib.h>
#include <string.h>

int token_clean_head_space(str *token)
{
	int i = 0;
	while (token->s[i] == ' '
			|| token->s[i] == '\n'
			|| token->s[i] == '\t') {
		i++;
		token->len -= 1;
	}
	token->s = &token->s[i];
	return 0;
}

int token_clean_tail_space(str *token)
{
	for (int i = token->len - 1; token->s[i] == ' '
			|| token->s[i] == '\n'
			|| token->s[i] == '\t'; i--) {
		token->len -= 1;
	}
	return 0;
}

int token_get_before(char c, str *token, str *result)
{
	int i = 0;
	result->s = token->s;
	while (token->s[i] != c) {
		if (i >= token->len)
			return 1;
		result->len += 1;
		i++;
	}
	token_clean_tail_space(result);
	token->s = &token->s[i];
	token->len -= result->len;
	return 0;
}

int token_get_list(const char *se, void *data, str *token,
		int (*func)(str *token, void *data))
{
	int ret = 0;
	str tmp = TOKEN_NEW;
	while (token->s[0] != se[1]) {
		if (token_get_before(se[0], token, &tmp))
			return 1;
		token_clean_head_space(&tmp);
		token_clean_tail_space(&tmp);
		ret = func(&tmp, data);
		if (ret)
			return ret + 1;
		token->s = &token->s[1];
		token->len -= 1;
	}
	return 0;
}

int token_get_token(str *token, str *result)
{
	result->s = token->s;
	for (int i = 0; i < token->len; i++) {
		if (token->s[i] != ' '
				&& token->s[i] != '\n'
				&& token->s[i] != '\t') {
			continue;
		}
		result->len = i;
		token->s = &token->s[i];
		token->len -= i;
		token_clean_head_space(token);
		return 0;
	}
	result->len = token->len;
	return 1;
}

int token_jump_to(char c, struct file *f)
{
	for (int i = 0; f->src[f->pos] != '\0'; i++) {
		if (f->src[f->pos] == c)
			return i;
		file_pos_next(f);
	}
	return -1;
}

int token_list_elem_end(char separator, struct file *f)
{
	if (f->src[f->pos] == separator) {
		file_pos_next(f);
		file_skip_space(f);
	} if (f->src[f->pos] == '\n') {
		file_line_next(f);
		file_skip_space(f);
	}
	return 0;
}

int token_next(str *token, struct file *f)
{
	if (token == NULL)
		return 1;
	if (file_check_end(f))
		return 1;
	token->s = &f->src[f->pos];
	while (f->src[f->pos] != ' ' && f->src[f->pos] != '\t') {
		if (file_check_end(f))
			return 0;
		if (f->src[f->pos] == '\n')
			return 0;
		token->len++;
		file_pos_next(f);
	}
	file_skip_space(f);
	return 0;
}

int token_parse_list(const char *se, void *data, struct file *f,
		int (*func)(const char *se, struct file *f, void *data))
{
	int ret = 0;
	while ((ret = func(se, f, data)) != -1) {
		if (ret > 0)
			return ret;
		if (f->src[f->pos] == se[1])
			return 0;
	}
	return 0;
}

char *token_read_before(const char *s, str *token, struct file *f)
{
	char *endc = NULL;
	token->s = &f->src[f->pos];
	while ((endc = strchr(s, f->src[f->pos])) == NULL) {
		token->len += 1;
		file_pos_next(f);
		if (f->src[f->pos] == '\0')
			return NULL;
	}
	token_clean_tail_space(token);
	return endc;
}

int token_read_region(char *se, str *region, struct file *f)
{
	int start_count = 0;
	file_pos_next(f);
	region->s = &f->src[f->pos];
	while (start_count != 0 || f->src[f->pos] != se[1]) {
		if (f->src[f->pos] == '\0')
			return 1;
		if (f->src[f->pos] == se[0])
			start_count++;
		if (start_count > 0 && f->src[f->pos] == se[1])
			start_count--;
		region->len += 1;
		file_pos_next(f);
	}
	return 0;
}

int token_try_read(str *expect, struct file *f)
{
	if (f->len - f->pos < expect->len)
		return 0;
	if (strncmp(&f->src[f->pos], expect->s, expect->len) != 0)
		return 0;
	file_pos_nnext(expect->len, f);
	file_skip_space(f);
	return 1;
}
