#include "die.h"
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void die(const char *msg, ...)
{
	va_list ap;

	va_start(ap, msg);
	vfprintf(stderr, msg, ap);
	va_end(ap);

	exit(1);
}

char *err_msg_get(const char *tok, int tok_len)
{
	char *msg = malloc(tok_len + 1);
	msg[tok_len] = '\0';
	memcpy(msg, tok, tok_len);
	return msg;
}
