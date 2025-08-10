/* This file is part of amc.
   SPDX-License-Identifier: GPL-3.0-or-later
*/
#include "include/label.h"
#include "../../utils/utils.h"
#include <stdio.h>
#include <string.h>

static int label_id_top = -1;

static int label_is_exists(label_id id);

int label_is_exists(label_id id)
{
	if (id > label_id_top)
		return 0;
	return 1;
}

label_id asf_label_alloc()
{
	label_id_top += 1;
	return label_id_top;
}

label_id asf_label_get_last()
{
	return label_id_top;
}

str *asf_label_get_str(label_id id)
{
	str *s = NULL;
	const char *temp = ".L%lld";
	if (!label_is_exists(id))
		goto err_label_not_found;
	s = str_new();
	str_expand(s, strlen(temp) - 3 + ullen(id));
	snprintf(s->s, s->len, temp, id);
	return s;
err_label_not_found:
	printf("amc[backend.asf]: asf_label_get_str: Label not found!\n");
	return NULL;
}
