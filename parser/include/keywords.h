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
int parse_const(struct file *f, struct symbol *sym, struct scope *scope);
int parse_elif(struct file *f, struct symbol *sym, struct scope *scope);
int parse_else(struct file *f, struct symbol *sym, struct scope *scope);
int parse_func_call(struct file *f, struct symbol *sym, struct scope *scope);
int parse_func_def(struct file *f, struct symbol *sym, struct scope *scope);
int parse_func_ret(struct file *f, struct symbol *sym, struct scope *scope);
int parse_if(struct file *f, struct symbol *sym, struct scope *scope);
int parse_let(struct file *f, struct symbol *sym, struct scope *scope);
int parse_match(struct file *f, struct symbol *sym, struct scope *scope);
int parse_struct(struct file *f, struct symbol *sym, struct scope *scope);
int parse_var(struct file *f, struct symbol *sym, struct scope *scope);
int parse_while(struct file *f, struct symbol *sym, struct scope *scope);

int keyword_end(struct file *f);
int keyword_find(str *token, struct symbol **result);

#endif
