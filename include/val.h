#ifndef AMC_VAL_H
#define AMC_VAL_H
#include "type.h"
#include "../utils/cint.h"

typedef struct yz_val {
	union {
		void *v;
		char *s;
		i8 b;
		i16 w;
		i32 i;
		i64 l;
	};

	yz_type type;
} yz_val;

typedef struct yz_extract_val {
	struct symbol *elem, *sym;
	union {
		int index;
		yz_val *offset;
	};
	enum {
		YZ_EXTRACT_ARRAY,
		YZ_EXTRACT_STRUCT
	} type;
} yz_extract_val;

struct symbol *yz_get_extracted_val(yz_extract_val *val);

void free_yz_extract_val(struct yz_extract_val *self);
void free_yz_val(yz_val *self);
void free_yz_val_noself(yz_val *self);

#endif
