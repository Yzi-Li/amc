#ifndef AMC_BACKEND_H
#define AMC_BACKEND_H
#include "file.h"
#include "backend/block.h"
#include "backend/expr.h"
#include "backend/func.h"
#include "backend/operator.h"

#define backend_call(FUNC) backends[cur_backend]->FUNC

enum BACKENDS {
	BE_NONE,
	BE_ASF
};

enum BE_STOP_SIGNAL {
	BE_STOP_SIGNAL_ERR,  // stop backend and notify backend has errors
	BE_STOP_SIGNAL_NULL, // no reason stop backend
	BE_STOP_SIGNAL_NE,   // no problem then stop backend
};

/**
 * Backend interface.
 * @note: control signal declaration in this file.
 */
struct backend {
	int (*end)();
	int (*file_new)(struct file *f);
	int (*init)(int argc, char *argv[]);
	int (*stop)(enum BE_STOP_SIGNAL bess);
	backend_block_end_f      block_end;
	backend_func_call_f      func_call;
	backend_func_def_f       func_def;
	backend_func_ret_f       func_ret;
	backend_op_cmd_f         ops[OP_LEN];
	backend_var_set_f        var_set;
	backend_var_immut_init_f var_immut_init;
};

extern struct backend *backends[];
extern enum BACKENDS cur_backend;

/**
 * Backend control functions.
 * @note: Backend control functions will block compiler operation.
 *        Don't do too time-consuming things.
 * @important: All control functions must be implemented.
 */

/**
 * Backend init.
 * @note: Arguments parse in selected backend.
 */
int backend_init(int argc, char *argv[]);

int backend_file_new(struct file *f);

/**
 * Notify the backend to stop compiling.
 * @important: All stop signals must be implemented.
 */
int backend_stop(enum BE_STOP_SIGNAL bess);

/**
 * Compiler completed, notify backend operation.
 * End backend normally.
 */
int backend_end();

#endif
