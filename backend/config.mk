BACKEND_SRC =\
	backend.c\
	object.c\
	target.c

BE_ASF_SRC =\
	asf.c\
	cmp.c\
	cond.c\
	expr.c\
	func.c\
	identifier.c\
	imm.c\
	jmp.c\
	label.c\
	loop.c\
	mov.c\
	op.c\
	op_add.c\
	op_assign.c\
	op_cmp.c\
	op_div.c\
	op_mul.c\
	op_ptr.c\
	op_sub.c\
	register.c\
	scope.c\
	stack.c\
	suffix.c\
	syscall.c

BACKEND_SRC += $(addprefix asf/, $(BE_ASF_SRC))

BACKEND_OBJ = $(BACKEND_SRC:.c=.o)
BACKEND_DEBUG_OBJ = $(BACKEND_SRC:.c=.debug.o)
