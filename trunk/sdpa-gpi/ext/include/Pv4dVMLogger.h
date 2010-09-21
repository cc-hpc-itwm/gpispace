#ifndef PV4D_VM_LOGGER_H
#define PV4D_VM_LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#ifdef USE_PV4D_LOGGER
#define printf	pv4d_printf
#endif

#ifdef __cplusplus
extern "C" {
#endif


void pv4d_printf(const char *fmt, ...);
void logMasterProcOutput();


#ifdef __cplusplus
}
#endif


#endif
