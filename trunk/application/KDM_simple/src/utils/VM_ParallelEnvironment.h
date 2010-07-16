/***************************************************************************
                          ParallelEnvironment.h  -  description

    Interface to the Parallel Environment for starting the parallel 
    process, allocation the global memory, reading local and remote
    data and stopping the parallel process. 
    This interface is implemented for MPI in MPI_ParallelEnvironment.cpp
    and for the VM in VM_ParallelEnvironment.cpp.
    Because of time-critical passage inheritence and polymorphism is 
    not used but the class is implemented in several flavours and chosen by linking.

                             -------------------
    begin                : Tue Jan 09 2007
    copyright            : (C) 2007 by Dirk Merten
    email                : merten@itwm.fhg.de

    change log:

      micheld | 2009-10-26
		syntax changes for VM4

      micheld | 2009-10-28
		cmdline should be NULL if not needed (empty char* will cause error)

 ***************************************************************************/


#ifndef PARALLELENVIRONMENT_H
#define PARALLELENVIRONMENT_H

#include <iostream>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

// fault tolerant pv4d and mctp are not implemented on Altivec
#ifdef __ALTIVEC__
#define VM3
#include "Pv4dVM3_IF.h"
#include "Pv4dVM4_bkw2_3.h"
#else
#include "Pv4dVM4.h"
#include "MCTP1.h"
#endif

#include "include/defs.h"

#include <sys/resource.h>




class ParallelEnvironment {

  static const vm_counter MUTEXCOUNTER = VMCounter0;
  static const vm_counter GENERALCOUNTER = VMCounter1;
  static const vm_counter BATCHCOUNTER = VMCounter2;
  static const vm_counter NODESLOTCOUNTER = VMCounter3;

 public:
    template<class T> class pgas_t
	{
	public:
	    /// pointer to local memory; different for each node
	    T *ptr;
	    /// local offset; identical on each node
	    unsigned long offset;
	};

// public methods
 public:
    ParallelEnvironment(){};
    /** Initialize VM. Starts binary given in cmdline on nodes given in configFile and allocate MemSize bytes on each machined. */
    void StartUp(int argc, char *argv[], 
	         const char *cmdline, 
	         const unsigned long MemSize, 
	         int& ret, 
	         const unsigned int GivenPort,
	         const bool PerformChecks = true);

    /** Shut down VM. Perform a Barrier and kill all remote processes. */
    ~ParallelEnvironment();

    /** Return Rank of process. */
    int GetRank();

    /** Return number of processes. */
    int GetNodeCount();

    /** Global barrier. */
    void Barrier();

    /** Fail Safe Barrier on responding nodes. */
    void FailSafeBarrier();

    /** Return pointer to allocated local memory. */
    void *getMemPtr();

    /** Parallel allocation of VM memory. */
    template<class T> pgas_t<T> Allocate(const unsigned long size)
	{
	    pgas_t<T> MemReference;
	    if ( free_offset + size < MemSize)
	    {
		MemReference.offset = free_offset;
		MemReference.ptr = (T*)((char*)MemPtr + free_offset);
		free_offset += size;
	    }
	    else
	    {
		MemReference.offset = 0;
		MemReference.ptr = NULL;
	    }
	    
	    return MemReference;
	};

    /** Return Size of allocated memory of process. */
    unsigned long GetMemSize(){return MemSize;};

    /** Non-blocking copy of size bytes from remote memory from process nodeID with offset remOffset to the local memory starting at offset loacalOffset using given queue.*/
    int readMem(const unsigned long localOffset,const unsigned long remOffset,const unsigned int size,
		 const unsigned int nodeID, const vm_queue queue);

    /** Non-blocking copy of size bytes from remote memory from process nodeID with offset remOffset to the local memory starting at offset loacalOffset using given queue.*/
    int writeMem(const unsigned long localOffset,const unsigned long remOffset,const unsigned int size,
		 const unsigned int nodeID,const vm_queue queue);

    /** Blocking send from local memory starting at offset loacalOffset to remote nodeID using given queue.*/
    int sendMem(const unsigned long localOffset, const unsigned int size, const unsigned int nodeID, const vm_queue queue);

    /** Blocking recieve from remote nodeID to local memory starting at offset loacalOffset using given queue.*/
    int recvMem(const unsigned long localOffset, const unsigned int size, const unsigned int nodeID, const vm_queue queue);

    /** Wait for queue to be finished */
    int WaitOnQueue(const vm_queue queue);

    /** Broadcast Size Bytes of Buffer from SrcRank to all other nodes; on receving nodes Buffer is allocated with new[] and Size is set appropriately */
    int Broadcast(char*& Buffer, int& Size, const int SrcRank);

    /** Increase global atomic counter by val and return old value*/
    unsigned int CounterAdd(unsigned int val);

    /** Reset global atomic counter*/
    int CounterReset();

    /** Init batch system globaly to start value 0.*/
    int InitBatch(const int StartVal = 0);

    /** Get next number from batch system or -1 for error. */
    int GetBatch();

    /** Set batch system globaly to error state. All other nodes will receive -1 on next call to GetBatch().*/
    int ReportErrorBatch();

    /** Set Mutex */
    void MutexLock(const unsigned int delay = 100);

    /** Free Mutex */
    void MutexUnlock();

    /** Set Node Slot */
    void NodeSlotLock(const unsigned int delay = 100);

    /** Free Node Slot */
    void NodeSlotUnlock();

    /** Free Node Slot */
    int NodeSlotReport();

    /** Finalize Node Slot */
    void NodeSlotFinalize();

    /** Blocking Send of size bytes starting at Var to process nodeID. */
    int Send(char* Var, const int size,
	      const unsigned int nodeID);
    /** Blocking Receive of size bytes tot Var from process nodeID. */
    int Receive(char* Var, const int size,
		 const unsigned int nodeID);

    /** Collect error codes from al nodes on rank 0 and exit if one of these != 0 **/
    void ExitOnError(const int err);

    /** Return true or false whether node nodeID is able to communicate **/
    bool GetNodeActivity(const int nodeID);

    /** Check for validity of the environemnt, return false and re-init on error **/
    bool Recover();

    /** Reserve a queue for exclusive communication in queue. Return true on success and false
        if no free queue is available. In case of failure the value of queue is not specified. **/
    bool ReserveQueue(vm_queue& qu);

    /** Mark Queue qu as free **/
    void ReleaseQueue(const vm_queue& qu);

    static int GetAllocatedNodeCount(){
	if ( !HostlistVMgenerated )
	{
	    NumberofAllocatedNodes = generateHostlistVM();
	    if ( NumberofAllocatedNodes != -1)
		HostlistVMgenerated = true;
	}
	return NumberofAllocatedNodes;
    };

// public attributes
 public:

// private methods
 private:
    void SetNodeActivity();

    ERROR_TYPE CheckEnvironment(const char* prgPath);

    ERROR_TYPE CheckRunningEnvironment();

    unsigned short FindPort(ERROR_TYPE& err_f );
// private attributes
 private:
    /// Pointer to local VM memory
    void *MemPtr;
    /// Size of local VM memory
    unsigned long MemSize;
    /// Maximum message length in bytes
    int MaxComSize;
    /// Maximum number of entries in each queue
    int QueueDepth;
    /// Rank of this process
    int rank;
    /// Rank of this process in ActiveRankList
    int rankActive;
    /// Number of nodes of the environment
    static int NumberofAllocatedNodes;
    /// Number of active nodes of the environment
    int ActiveNodeCount;
    /// List of ranks of active nodes
    int* ActiveRankList;
    /// List of activity of all nodesranks of active nodes
    bool* NodeActivity;
    /// Flag for successfull start-up
    bool StartSuccess;

    /// Number of queues available
    int NumberOfQueues;
    /// Vector of enum names of the queues
    vm_queue queue_name[8];
    /// Vector of queue reservation status. true: reserved, false: free
    bool queue_reservation[8];

    unsigned long free_offset;

    static bool HostlistVMgenerated;
    
};
#endif  // PARALLELENVIRONMENT_H
