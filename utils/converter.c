#include "converter.h"

int str2int(str *s, long long *l)
{
	if (s->len <= 0)
		return 1;
	for (int i = 0; i < s->len; i++) {
		if (s->s[i] < '0' || s->s[i] > '9')
			return 2;
		*l *= 10;
		*l += s->s[i] - '0';
	}
	return 0;
}
