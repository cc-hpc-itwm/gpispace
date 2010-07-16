#ifndef PV4D_LOGGER_H
#define PV4D_LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#ifdef USE_FVM_LOGGER
#define printf	fvm_printf
#endif

#ifdef __cplusplus
extern "C" {
#endif


void fvm_printf(const char *fmt, ...);



#ifdef __cplusplus
}
#endif


#endif
