#ifndef _FVM_SYNC_H_
#define _FVM_SYNC_H_

#include <pthread.h>
#include <netinet/in.h>
#include <sys/select.h>

/* Implementation of a syncronization mechanism similar to condition
   variables and mutexes in a global fashion that is, for all nodes in
   FVM 
         and
   communication synchronization (wait, allow, check if allowed) */

/* Condition Variables */

typedef struct {
  int sock;
  struct sockaddr_in addr;
  int port;
  char *ip;
 
} fvmCondVar_t;

typedef pthread_mutex_t fvmMutex_t;


static pthread_mutex_t mutex_communication;
static pthread_cond_t cond_communication;

int fvmMutexInit(fvmMutex_t *fvm_mutex);
int fvmMutexDestroy(fvmMutex_t *fvm_mutex);
int fvmCondInit(fvmCondVar_t *fvm_var);
int fvmCondWait(fvmCondVar_t *fvm_var, fvmMutex_t *fvm_mutex, const char *sender);
int fvmCondSignal(fvmCondVar_t *fvm_var);
int fvmCondDestroy(fvmCondVar_t *fvm_var);

int waitAllCommunication();

void deferCommunication();

void allowCommunication();

int isCommAllowed();

void waitCommAllowed();

#endif
