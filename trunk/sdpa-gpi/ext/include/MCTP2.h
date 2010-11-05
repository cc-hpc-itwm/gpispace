#ifndef MCTP2_H
#define MCTP2_H

#include <pthread.h>
#include <semaphore.h>

#define ALIGN64  __attribute__ ((aligned (64)))
#define MCTP2_MAX_THREADS (128)
#define MCTP2_MAX_LOCKS   (16)
#define MCTP2_MAX_POOLS   (4)

//mc-extension
#define MCTP2_ENUM_ALL (0xffffffff)

//data structures, used only internally
typedef struct{
  int               magic;
  int volatile      counter;
  int volatile      release;
  pthread_mutex_t   lock;
}mctp2Barrier,*mctp2BarrierPtr;


typedef struct{
  pthread_t hID;
}mctp2ThreadID;


typedef struct{
  ALIGN64  int futex_event;
  unsigned int futex_still_needed;
  unsigned int futex_initial_needed;
}mctp2PoolSync;

//return values: -1 on error, 0 on success

/* The mctp2 class (MultiCore Thread Pools) provides a pool of threads.
 * The threads in a pool can be synchronized with each other, and a pool
 * can be synchronized to another pool.
 */
class mctp2{

public:


  // Creates an instance of the mctp2 class.
  mctp2();

  // mctpCreatePool must be called after the object has been created.
  // It sets the number of cores this thread pool should use. This value
  // should correspond in some way with the number of available CPU cores.
  // If the default parameter 0 is used, the number of cores will be set
  // automatically to the number of total (physically and logically) existing CPU cores.
  int mctpCreatePool(unsigned int cores = 0);
  // Returns the unique id for this thread pool. The ids are assigned internally.
  int mctpGetPoolID();
  // Returns the number of cores activated using the mctpCreatePool() call.
  int mctpGetNumberOfActivatedCores();
  // Cleanup internal data-structures, should be called when all work is done
  // with this mctp2 instance before the object is destructed.
  void mctpCleanup();


  // Once a thread has been started via mctpStartThread(), the first thing this
  // new thread should do in its thread function is to call mctpRegisterThread().
  // It returns the id of the calling thread. This id is unique per thread pool.
  //
  // Also the initial thread (main()) of the process must be part of a thread pool.
  // If it should not be in one of the thread groups as the other threads, one
  // may create a separate "dummy" thread pool with one thread just for main().
  int mctpRegisterThread();
   
  //thread management

  // Start a thread, it will execute the given function.
  void mctpStartThread(void* (*function)(void*),void *arg);
  // Suspend the given thread, immediately and independent from any other
  // synchronization primitives as mutexes, semaphores, etc., i.e. it can
  // also put threads currently holding mutexes to sleep.
  // Use with care.
  int  mctpSuspendThread(const int tID);
  // Resume the given thread.
  int  mctpResumeThread(const int tID);


  // Synchronization
  void mctpSyncPoolWith(mctp2 *otherPool);

  // Synchronize all threads in this pool (barrier).
  // Any of the threads in this group can only continue after all threads
  // of the pool have reached this point.
  // This one is implemented using busy waiting.
  void mctpSyncThreads();
  // Same as above, but the waiting is implemented differently.
  // It also uses busy waiting, but each waiting thread issues PAUSE CPU
  // instructions, which cause that CPU core to do nothing for several cycles,
  // and by that not occupy the memory bus by constantly polling the shared variable.
  void mctpSyncThreadsRelaxed();
  // Same as above, but implemented using mutexes (futexes), i.e. not busy waiting.
  void mctpSyncThreadsPassive();
  

  //image tile to render (rt option)
  long mctpGetTileID();
  void mctpClearTileCounter();

private:

  //methods
  int mctpInit();
  int mctpInitUser(const unsigned int cores);
  
  void mctp_futex_init(const int nr);
  void mctp_futex_cleanup();
  int  mctp_barrier_init(mctp2BarrierPtr *br);
  int  mctp_barrier_destroy(mctp2BarrierPtr *b);
  
  void mctpInsertThread2Map(pthread_t tid);
  int  mctpGetNumberOfCoresInternal();
  void mctpRegisterSigHandlers();

private:

  //data
  int poolID;
  int totalElem;
  
  unsigned int futex_still_needed;
  unsigned int futex_initial_needed;
  pthread_mutex_t futex_mutex0;
    
  pthread_mutex_t  mctpStatusLock; 
  pthread_mutex_t  mctpGlbFutexLock;
  mctp2BarrierPtr  mctp_thread_ba1;
  
  
  volatile long mctpGlobalIDCnt;
  volatile long mctpThreadStatus[MCTP2_MAX_THREADS];
  mctp2ThreadID glbThreadID[MCTP2_MAX_THREADS];
  int MCTP2_NUM_THREADS;

  int futex_event;
  sem_t mctpThreadUp;
  long mctp_atomic_tile_cnt;

};//end of class


//high resolution timer
typedef unsigned long cycles_t;

// The class hrTimer provides a high resolution timer for
// measurement of time intervals down to usecs.
class hrTimer{

public:

// Create an instance of the hrTimer class
hrTimer();

// Initialize the hrTimer object. Must be called before any of the other
// timer-functions can be called (they return invalid values otherwise).
void   Init();
// (Re)Start the timer for a new measurement.
void   Start();
// Stop the timer, i.e. store the time elapsed since the last call to mctp2StartTimer().
// This can be called multiple times after one mctp2StartTimer() call.
void   Stop();
// Returns the time (in seconds) elapsed between the last mctp2StartTimer() call
// and the last call to mctp2StopTimer().
double GetSecs();
// Returns the time (in milliseconds) elapsed between the last mctp2StartTimer() call
// and the last call to mctp2StopTimer().
double GetMSecs();
// Returns the measured CPU frequency.
double GetCPUFreq();
// Set a CPU frequency which will be used for calculations by the timer
// Does not physically change the CPU frequency.
void   SetCPUFreq(const double MHz);

private:

cycles_t tposted,tcompleted;
double cycles_to_secs,cycles_to_msecs;

};


//mctp2 version
float mctp2GetVersion();

//mctp2 capabilities
int mctp2GetMaxPools();
int mctp2GetMaxThreads();
int mctp2GetMaxLocks();


// Old deprecated Thread affinity, max cores=64

// Returns the affinity mask of the calling thread.
void mctp2GetAffinityMask(unsigned long *mask);
// Sets the affinity mask of the calling thread.
int  mctp2SetAffinityMask(const unsigned long mask);
// Set the affinity mask of the calling thread so that it will be bound to
// the CPU with the given number nr.
void mctp2Thread2CoreAffinity(unsigned int nr);//[0..63]


// New MC-Extension

//only available/activated on modern procs
int mctp2InitMCExtension();
int mctp2CleanupMCExtension();
int mctp2HasHT();
int mctp2EnableHT();//default: enabled if available
int mctp2DisableHT();
int mctp2GetSocketCount();
int mctp2GetPhyCoreCount();
int mctp2GetThreadsPerCore();
int mctp2SetSocketAffinity(const unsigned int socket);
int mctp2SetGenericAffinityMask(const unsigned int socket,const unsigned int core,const unsigned int htThread);
int mctp2GetCpu();


//new thread save/cacheline aligned mallocCA
void *mallocCA(size_t size);
void freeCA(void *ptr);
//simple atomic counting of mallocCA/freeCA
int mctp2GetMallocCASize();

//linux filecache operations (option io)
int mctp2ReleaseFC(const int fd,const unsigned long offset);


//sse support
bool mctp2CheckSSE1();
bool mctp2CheckSSE2();
bool mctp2CheckSSE3();

//4 or above = SSE2 supported
//5 or above = SSE3 supported
//6 or above = Supplementary SSE3 supported
//7 or above = SSE4.1 supported
//8 or above = SSE4.2 supported
int  mctp2_get_sse_level();
char *mctp2_get_sse_str(const int l);

bool mctp2CheckMWait();
void mctp2Sleep(const int msec);



//critical sections (locks)
// There are to MCTP2_MAX_LOCKS locks (spinlocks) available, with
// lockNr from 0..(MCTP2_MAX_LOCKS-1).
void mctp2Lock(const unsigned char lockNr);
void mctp2Unlock(const unsigned char lockNr);


// Atomics

// Atomically return the value from the given address, and add val
// to the memory location after returning it.
unsigned long mctp2_fetch_and_add_addr(void *addr,const long val);
// Atomically compare and exchange. If the value at the given address
// is equal to cmpVal, set it to val, otherwise do nothing.
void mctp2_cmpxchg_addr(void *addr,const long cmpVal,const long val);
unsigned long mctp2_fetch_and_nop_addr(void *addr);
// Set the value at the given location to 0.
void mctp2_atomic_clear_addr(void *addr);


// System hardware, returns the number of of cores.
int mctp2GetNumberOfCores();

#endif
