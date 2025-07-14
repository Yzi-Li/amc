#ifndef AMC_BE_OBJECT_H
#define AMC_BE_OBJECT_H
#include "../../utils/str/str.h"

extern struct object_head *cur_obj;

struct object_node;
struct object_node {
	str *s;
	struct object_node *next;
	struct object_node *prev;
};

struct object_section {
	struct object_node *head;
	struct object_node *last;
};

struct object_head {
	struct object_head *prev;
	struct object_section *sections;
	int sec_count;
};

int object_append(struct object_section *sec, struct object_node *n);
/**
 * Insert object_node between 'n1' and 'n2'.
 */
int object_insert(struct object_node *src, struct object_node *n1,
		struct object_node *n2);
int object_remove(struct object_section *sec, struct object_node *n);
int object_remove_last(struct object_section *sec);
int object_swap(struct object_node *n1, struct object_node *n2);
void object_head_free(struct object_head *h);
void object_section_free(struct object_section *sec);

#endif
