#ifndef AMC_BE_SYMBOL_H
#define AMC_BE_SYMBOL_H

typedef void backend_symbol_status;

typedef void (*backend_symbol_status_free_f)(backend_symbol_status *raw_stat);

#endif
