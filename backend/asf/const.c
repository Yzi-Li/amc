#include "include/asf.h"
#include "../../include/backend/object.h"
#include <stdlib.h>
#include <string.h>

static int const_count = -1;
static const char *const_label = ".LC%d:\n";

static int const_label_new();

int const_label_new()
{
	struct object_node *node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_RODATA], node))
		return 1;
	node->s = str_new();
	const_count += 1;
	str_expand(node->s, strlen(const_label) - 1
			+ ullen(const_count));
	snprintf(node->s->s, node->s->len, const_label, const_count);
	return 0;
}

int asf_const_def_str(backend_const *self, str *s)
{
	const char *temp = ".string \"%s\"\n";
	struct object_node *node = NULL;
	if (s == NULL)
		return 1;
	if (const_label_new())
		return 1;
	node = malloc(sizeof(*node));
	if (object_append(&cur_obj->sections[ASF_OBJ_RODATA], node))
		return 1;
	node->s = str_new();
	str_expand(node->s, strlen(temp) + s->len);
	snprintf(node->s->s, node->s->len, temp, s->s);
	self->val.type.type = YZ_I32;
	self->val.i = const_count;
	return 0;
}
