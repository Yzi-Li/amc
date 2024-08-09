#include "suffix.h"

char suffix_get(int bytes)
{
	switch (bytes) {
	case 1:
		return 'b';
		break;
	case 2:
		return 'w';
		break;
	case 4:
		return 'l';
		break;
	case 8:
		return 'q';
		break;
	}

	return '\0';
}
