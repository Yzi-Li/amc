#ifndef AMC_BE_IDENTIFIER_H
#define AMC_BE_IDENTIFIER_H
#include "../../../include/backend/scope.h"

void asf_identifier_free_id(int num);
struct asf_stack_element *asf_identifier_get(char *name);
int asf_identifier_reg(char *name, struct asf_stack_element *src,
		backend_scope_status *raw_status);

#endif
