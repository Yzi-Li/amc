BACKEND_SRC =\
	backend.c\
	target.c

BE_ASF_SRC =\
	asf.c\
	expr.c\
	func.c\
	imm.c\
	mov.c\
	op.c\
	op_add.c\
	op_div.c\
	op_mul.c\
	op_sub.c\
	register.c\
	stack.c\
	suffix.c

BACKEND_SRC += $(addprefix asf/, $(BE_ASF_SRC))

BACKEND_OBJ = $(BACKEND_SRC:.c=.o)
BACKEND_DEBUG_OBJ = $(BACKEND_SRC:.c=.debug.o)
