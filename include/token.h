/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#ifndef AMC_TOKEN_H
#define AMC_TOKEN_H
#include "../utils/str/str.h"
#include "../include/file.h"

#define TOKEN_NEW {.len = 0}

int token_clean_head_space(str *token);
int token_clean_tail_space(str *token);

/**
 * Get a token before 'c'.
 * @note: It won't include ending character.
 */
int token_get_before(char c, str *token, str *result);

/**
 * Parse a list from token.
 * @param se: separator and end.
 */
int token_get_list(const char *se, void *data, str *token,
		int (*func)(str *token, void *data));

/**
 * Get a token from token.
 */
int token_get_token(str *token, str *result);

/**
 * Jump to specified character.
 * @return:
 *   Number of characters passed through.
 *   -1: end of file and not found specified character.
 */
int token_jump_to(char c, struct file *f);

/**
 * Handle end of element of 'token_parse_list'
 * @note:
 *   Use it on element of 'token_parse_list'.
 * @return: always 0. so use it in return.
 */
int token_list_elem_end(char separator, struct file *f);

/**
 * Get next token from file.
 * Don't jump to next line.
 */
int token_next(str *token, struct file *f);

/**
 * Parse a list from file.
 * @param se: separator, end character.
 * @param func:
 *   @important: Handle 'se' parameter in 'func'.
 *   @param se: separator, end character.
 */
int token_parse_list(const char *se, void *data, struct file *f,
		int (*func)(const char *se, struct file *f, void *data));

/**
 * Read a token from file.
 * @return: end character.
 */
char *token_read_before(const char *s, str *token, struct file *f);

/**
 * Read a region in se[0] and se[1].
 * @param se: start character and end character.
 */
int token_read_region(char *se, str *region, struct file *f);

int token_try_read(str *expect, struct file *f);

#endif
