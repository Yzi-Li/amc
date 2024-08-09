#ifndef AMC_BE_TARGET_H
#define AMC_BE_TARGET_H
#include "../../utils/str/str.h"

extern struct object_head **objs;
extern int cur_obj;

struct object_node;
struct object_node {
	str *s;
	struct object_node *next;
	struct object_node *prev;
};

struct object_head {
	struct object_node *head;
	struct object_node *last;
};

int object_append(struct object_head *h, struct object_node *n);
/**
 * Insert object_node between 'n1' and 'n2'.
 */
int object_insert(struct object_node *src, struct object_node *n1,
		struct object_node *n2);
void object_free(struct object_head *o);
int target_write(struct object_head **objs, int len);

#endif
