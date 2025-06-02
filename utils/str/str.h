#ifndef STR_H
#define STR_H

typedef struct str {
	int len;
	char *s;
} str;

char *str2chr(const char *s, int len);
int str_append(str *src, int len, const char *s);
int str_expand(str *src, int len);
void str_free(str *src);
str* str_new();

#endif
