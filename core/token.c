/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "../include/token.h"
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
