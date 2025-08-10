BACKEND_SRC =\
	backend.c\
	object.c\
	target.c

BE_ASF_SRC =\
	array.c\
	asf.c\
	bytes.c\
	cmp.c\
	call.c\
	cond.c\
	const.c\
	expr.c\
	file.c\
	func.c\
	identifier.c\
	imm.c\
	jmp.c\
	label.c\
	loop.c\
	mov.c\
	null.c\
	op.c\
	op_add.c\
	op_cmp.c\
	op_div.c\
	op_mul.c\
	op_ptr.c\
	op_sub.c\
	op_val.c\
	register.c\
	scope.c\
	stack.c\
	struct.c\
	suffix.c\
	symbol.c\
	val.c

BACKEND_SRC += $(addprefix asf/, $(BE_ASF_SRC))

BACKEND_BUILD = ../build/backend
BACKEND_OBJ = $(addprefix $(BACKEND_BUILD)/, $(BACKEND_SRC:.c=.o))
