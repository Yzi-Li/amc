#ifndef AMC_PARSER_KEYWORDS_H
#define AMC_PARSER_KEYWORDS_H
#include "../../include/file.h"
#include "../../include/symbol.h"

/*
 * Actually, this is the where all parse function definitions are located.
 */

/**
 * @return:
 *   0: if not a comment.
 *   1: if single line comment.
 */
int parse_comment(struct file *f);
int parse_const(struct parser *parser);
int parse_elif(struct parser *parser);
int parse_else(struct parser *parser);
int parse_func_call(struct parser *parser);
int parse_func_def(struct parser *parser);
int parse_func_ret(struct parser *parser);
int parse_if(struct parser *parser);
int parse_let(struct parser *parser);
int parse_match(struct parser *parser);
int parse_mod(struct parser *parser);
int parse_struct(struct parser *parser);
int parse_var(struct parser *parser);
int parse_while(struct parser *parser);

int keyword_end(struct file *f);
int keyword_find(str *token, struct symbol **result);

#endif
