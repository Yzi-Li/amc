#ifndef AMC_STRUCT_H
#define AMC_STRUCT_H

struct yz_struct_flag {
	unsigned int mut:1, rec:1;
};

struct symbol;
typedef struct yz_struct {
	struct symbol **elems;
	int elem_count;
	struct yz_struct_flag flags;
	char *name;
} yz_struct;

void free_struct(yz_struct *src);
void free_structs(yz_struct **elems, int count);

#endif
