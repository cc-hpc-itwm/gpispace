#ifndef MC_THREADP_H
#define MC_THREADP_H

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif


#ifndef WIN32
typedef unsigned long cycles_t;
#else
typedef __int64 cycles_t;
#endif


//return values: -1 on error, 0 on success

//# available system cores
int mctpGetNumberOfCores();

//take all cores available
void mctpInit();
//user specified core count
void mctpInitUser(const unsigned int use_nr_of_cores);
//finalize everything
void mctpCleanup();
//current mctp1 version
float mctpGetVersion();
//register mctp thread (must be first thread call)
int mctpRegisterThread();
//sync all used threads
void mctpSyncThreads();
void mctpSyncThreadsRelaxed();
void mctpSyncThreadsPassive();

//image tile to render (rt option)
int mctpGetTileID();
void mctpClearTileCounter();

//64bit atomics
long mctp_fetch_and_add_addr(void *addr,const long val);
void mctp_cmpxchg_addr(void *addr,const long cmpVal,const long val);
long mctp_fetch_and_nop_addr(void *addr);
void mctp_atomic_clear_addr(void *addr);

//simple thread startup, no group operations like: barrier,start,stop, etc.
void mctpStartSingleThread(void* (*function)(void*),void *arg);

//thread ctrl
void mctpStartThread(void* (*function)(void*),void *arg);
int  mctpSuspendThread(const int tID);
int  mctpResumeThread(const int tID);
void mctpSleep(const int msec);

//affinity mask (max 64 cores) can be calculated with utility: systemTopology.exe
void mctpGetAffinityMask(unsigned long *oldmask);
int  mctpSetAffinityMask(const unsigned long affinityMask);
void mctpThread2CoreAffinity(unsigned int nr);//starting from [0..N]

int mctpGetNumberOfActivatedCores();

//sse support
int mctpCheckSSE1();
int mctpCheckSSE2();
int mctpCheckSSE3();

//4 or above = SSE2 supported
//5 or above = SSE3 supported
//6 or above = Supplementary SSE3 supported
//7 or above = SSE4.1 supported
//8 or above = SSE4.2 supported
int  mctp_get_sse_level();

char *mctp_get_sse_str(const int l);
int mctpCheckMWait();

//new thread save/aligned malloc
void *mallocCA(size_t size);
void freeCA(void *ptr);
//simple atomic counting of mallocCA/freeCA pairs
int mctpGetMallocCASize();


//critical sections (locks)
void mctpLock(const unsigned char lockNr);
void mctpUnlock(const unsigned char lockNr);

//linux filecache operations
int mctpReleaseFC(const int fd,const unsigned long offset);

//high resolution timer
void mctpInitTimer();
void mctpStartTimer();
void mctpStopTimer();
double mctpGetTimerSecs();
double mctpGetTimerMSecs();
void   mctpSetCPUFreq(const double MHz);
double mctpGetCPUFreq();
//plain cpu cycles
cycles_t get_cycles();

//mctp capabilities
int mctpGetMaxThreads();
int mctpGetMaxLocks();

#ifdef __cplusplus
}
#endif

#endif
