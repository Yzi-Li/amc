#include "str.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *str2chr(const char *s, int len)
{
	char *result = malloc(len + 1);
	result[len] = '\0';
	memcpy(result, s, len);
	return result;
}

int str_append(str *src, int len, const char *s)
{
	int last;
	if (src == NULL)
		return 1;
	last = src->len;
	str_expand(src, len);
	memcpy(&src->s[last], s, len);
	return 0;
}

int str_expand(str *src, int len)
{
	if (src == NULL)
		return 1;
	src->s = realloc(src->s, src->len + len);
	src->len += len;
	return 0;
}

void str_free(str *src)
{
	if (src == NULL)
		return;
	free(src->s);
	src->s = NULL;
	src->len = 0;
}

str *str_new()
{
	str *s = malloc(sizeof(*s));
	s->s = NULL;
	s->len = 0;
	return s;
}
