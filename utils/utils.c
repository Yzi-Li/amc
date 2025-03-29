#include "utils.h"

#define AMC_INT_LEN_FUNC(NAME, TYPE) \
	int NAME(TYPE src) { \
		if (src == 0)\
			return 1;\
		int len = 0; \
		for (TYPE i = src; i != 0; len++) { \
			i /= 10; \
		} \
		return len; \
	}

int checkint(const char *str, int len)
{
	for (int i = 0; i < len; i++) {
		if (str[i] < '0' || str[i] > '9')
			return 0;
	}
	return 1;
}

AMC_INT_LEN_FUNC(ublen, unsigned char)
AMC_INT_LEN_FUNC(uslen, unsigned short)
AMC_INT_LEN_FUNC(uilen, unsigned int)
AMC_INT_LEN_FUNC(ullen, unsigned long long)
