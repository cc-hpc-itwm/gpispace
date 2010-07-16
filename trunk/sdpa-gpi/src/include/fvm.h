#ifndef _FVM_H_
#define _FVM_H_

#include <stdio.h>
#include <unistd.h>
#include <errno.h>


#include <Pv4dVMLogger.h>
#include <Pv4dVM4.h>

#include "fvmConfig.h"
#include "fvmAllocator.h"
#include "fvm_common.h"


const char *op2str(fvmOperation_t op);
int fvmListenRequests(void);
int fvmWait4PC(configFile_t);
int fvmInit(configFile_t);
int fvmLeave(void);
#endif
