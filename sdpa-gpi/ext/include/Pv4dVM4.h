/*! \file Pv4dVM4.h
    \brief The interface of the Fraunhofer Virtual Machine.
*/

#ifndef PV4D_VM_H
#define PV4D_VM_H

#ifdef __cplusplus
extern "C" {
#endif

  //! \name Types
  /*@{*/
  /*! \enum vm_queue
   * The available queues for communication
   */
  enum vm_queue   {VMQueue0, VMQueue1, VMQueue2, VMQueue3, VMQueue4, VMQueue5, VMQueue6, VMQueue7};

  /*! \enum vm_counter
   * The available global atomic counters
   */
  enum vm_counter {VMCounter0, VMCounter1, VMCounter2, VMCounter3, VMCounter4, VMCounter5, VMCounter6, VMCounter7};

  /*! \typedef VM_OP
   * The available operations with collectives
   */
  typedef enum { VM_MIN=0, VM_MAX=1, VM_SUM=2 } VM_OP;

  /*! \typedef VM_TYPE
   * The available data types to work with collectives
   */
  typedef enum { VM_INT=0, VM_UINT=1, VM_FLOAT=2, VM_DOUBLE=3, VM_LONG=4, VM_ULONG=5 } VM_TYPE;

  /*@}*/

  //! \name Infos and Capabilities
  /*@{*/
  //! Get the number of available queues for communication
  /*!
    \return an int with the number of available queues
  */

  int getNumberOfQueuesVM(void);

  //!brief Get the available queue depth
  /*!
    \return an int with the queue depth

    The queue depth is the number of requests that can be posted to a queue without calling a waitDmaVM on it.
  */
  int getQueueDepthVM(void);


  //! Get the available global atomic counters
  /*!
    \return an int with the number of global atomic counters

    The number of global atomic counters is limited to the available number.
    They can be referenced as a int value or using the enum vm_counter.
  */
  int getNumberOfCountersVM(void);

  //!brief Returns the FVM version.
  /*!

    \return a float with the version.
  */
  float getVersionVM();

  //! Get the port used by the FVM in internal communication with the daemon.
  /*!

    \return an unsigned short with the port number.

    Knowing the port number might be useful to track any
    communication problems with the daemon (e.g. in a system that blocks some ports).
    The default value is 10820.
  */
  unsigned short getPortVM();

  //! Set the port used by the FVM in internal communication with the daemon.
  /*!

    \param port The port number to use
    \return an int where 0 is success and -1 is failure.

    Setting the port number to a particular value might be needed if
    the default value (10820) is already in use or blocked.
  */
  int setPortVM(const unsigned short port);


  //!Set the MTU size for transfer.
  /*!
    \param mtu The MTU size to use.

    \return an int where 0 is success and -1 is failure.

    Sets the MTU size for the communication (readDmaVM, writeDmaVM,
    sendDmaVM, recvDmaVM, etc.).

    The default value is 1024 but one should use 2048 and above on
    modern cards. This brings a performance boosts on communication.
  */

  int setMtuSizeVM(const unsigned int mtu);

  //! Set the number of processes to start the FVM.
  /*!
    \param np The number of processes.

    Setting the number of processes is equivalent to setting the
    number of nodes where to run the application. By default the FVM
    will start the same number of processes as the number of nodes on
    the machine file. If the application should run on less nodes
    than that number, this function should be used.
  */

  void setNpVM(const unsigned int np);

  //! Get the total number of nodes.
  /*!

    \return Returns the total number of nodes or -1 on call failure.
  */
  int generateHostlistVM();

  //! Get the hostname of a node
  /*!
    \param rank The node rank to get the hostname.

    \return a char pointer with the hostname.
  */
  const char *getHostnameVM(const unsigned int rank);
  /*@}*/


  //! \name Environment runtime check
  /*@{*/
  //! Ping the daemon on a certain node
  /*!
    \param hostname The hostname where to ping the daemon.

    \return an int where 0 is success and -1 is failure.

    Ping the daemon to find out if the daemon is running on that
    particular hostname.
  */

  int pingPv4dDaemonVM(const char *hostname);


  //! Tells if the calling process is the master process
  /*!
    \param argc The argc of the process
    \param argv The argv of the process

    \return an int where 1 is success, 0 is failure and -42 is timeout.
  */

  int isMasterProcVM(int argc, char *argv[]);

  //! Check if all shared libraries are found.
  /*!

    \param hostname The hostname where to do the check.
    \param prgPath The path to the application binary.
    \return an int where 0 means success, -1 is operation failed, -2 means missing shared libs and -42 is timeout.

    The application runs on a clean environment and therefore some
    libraries might not be found as in the master node. This function
    ensures that everything is OK.
  */

  int checkSharedLibsVM(const char *hostname, const char *prgPath);

  //! Check if port is in use.
  /*!
    \param hostname The hostname where to do the check.
    \param portNr The port number to check.
    \return an int where 1 is success, 0 is failure and -42 is timeout.


    Check if the given port number is available to use.
  */

  int checkPortVM(const char *hostname, const unsigned short portNr);

  //! Find if the program started by daemon really up and running.
  /*!
    \param hostname The hostname where to do the check.
    \return an int where 1 is success, 0 is failure and -42 is timeout.
  */

  int findProcVM(const char *hostname);

  //! Clear the Linux file cache.
  /*!
    \param hostname The hostname where to execute the clear.
    \return an int where 1 is success, 0 is failure and -42 is timeout.

  */
  int clearFileCacheVM(const char *hostname);

  //! Check if Infiniband is running properly.
  /*!
    \param hostname The hostname where to run the check.
    \return an in where 0 is success, -1 is operation failed, -2 is no active port/link and -42 is timeout
  */
  int runIBTestVM(const char *hostname);

  /*@}*/


  //! \name Start and stop FVM
  /*@{*/
  //! Start the FVM
  /*!
    \param argc Argument count
    \param argv Command line rguments
    \param cmdline The command line to be used to start the binaries
    \param vmMemSize The memory space allocated for the FVM

    \return an int where -1 is operation failed, -42 is timeout and 0 is success.

    \warning The command line arguments (argc, argv) won't be forward to the worker nodes

  */
  int startPv4dVM(int argc,char *argv[],const char *cmdline,const unsigned long vmMemSize);

  //! Shutdown the FVM.
  /*!
    \warning This only happens on a node that calls this function. It is not a collective request.

    Cleanly, bring all infrastructure down and releasing resources on the node.
  */
  void shutdownPv4dVM();

  //! Kill the all running FVM processes.
  /*!
    \warning This is a function to be called by the master node
    \return an int where 0 is success and -1 is failure.

    Kills the application on all nodes to avoid hanging
    applications. This should be called on error or failure
    situations.
  */
  int killProcsVM(void);
  /*@}*/


  //! \name Node utilities
  /*@{*/
  //! Get the node rank.
  /*!
    \return an int with the rank value.

    The rank is the identifier of a node and unique to each node.
  */

  int getRankVM(void);

  //! Get the number of nodes
  /*!

    \return an int with the number of nodes.
  */
  int getNodeCountVM(void);

  //! Get the memory reserved for the FVM operation on a worker node.
  /*!

    \return unsigned long with the memory size.

    The Worker node memory size might be different than the Master
    node (e.g. when diferent binaries are started).
  */
  unsigned long getMemWorkerVM(void);

  //! Get the pointer to the FVM global memory available for DMA operations.
  /*!
    \return void pointer to the fvm memory start
  */
  void *getDmaMemPtrVM(void);

  /*@}*/

  //! \name Barriers
  /*@{*/
  //! Execute a barrier over the socket infrastructure.
  /*!
    \return void.
  */
  void socketBarrierVM(void);

  //! Execute a fast barrier.
  /*!
    \return void.
  */
  void pv4dBarrierVM(void);

  /*@}*/

  //! \name Collectives
  /*@{*/
  //! Allreduce collective operation of a vector of elements (up to 255)
  /*!

    \param sendBuf A pointer to the buffer containing the value.
    \param recvBuf A pointer to the buffer where the result should be placed.
    \param elemCnt The number of elements of the vector
    \param op The type of operation to perform (see enum VM_OP).
    \param type The datatype of the values to perform the operations (see enum VM_TYPE).

    \return an int where -1 is operation failed and 0 is success.

    The sendBuf and recvBuf don't have to be on the FVM global memory
    regions. Heap or stack memory locations can also be used.

  */
  int allReduceVM(void *sendBuf,void *recvBuf,const unsigned char elemCnt, VM_OP op, VM_TYPE type);

  /*@}*/

  //! \name Socket Communication
  /*@{*/
  //! Send data via socket infrastructure.
  /*!
    \param buffer The buffer to send
    \param len Length of the buffer to send
    \param rank The rank where to send
    \warning Data cannot be sent to the localhost
    \warning This call is non-blocking.

    \return an int where -1 is operation failed and 0 is success.
  */

  int sendSocketVM(void *buffer,const int len,const unsigned int rank);

  //! Receive data via socket infrastructure.
  /*!
    \param buffer The buffer where to receive
    \param len Length of the buffer to receive
    \param rank The rank from where to receive
    \warning Data cannot be sent to the localhost
    \warning This call is blocking.

    \return an int where -1 is operation failed and 0 is success.
  */

  int recvSocketVM(void *buffer,const int len,const unsigned int rank);

  //! Non-blocking receive data via socket infrastructure.
  /*!
    \param buffer The buffer where to receive
    \param len Length of the buffer to receive
    \param rank The rank from where to receive
    \warning Data cannot be sent to the localhost
    \warning This call is non-blocking.

    \return an int where -1 is operation failed and 0 is success.
  */

  int recvSocketNbVM(void *buffer, const int len, const unsigned int rank);


  //! Wait for any data on the socket infrastructure.
  /*!

    \return int with the number of received messages and -1 on failure.

  */
  int waitOnAnySocketDataVM();

  /*@}*/

  //! \name DMA communication
  /*@{*/
  //! Read data from a remote node memory location
  /*!

    \param localOffset the offset on the node where the data to be transfered whould be written
    \param remOffset the offset on the remote node where the data should be read from
    \param size the size of the transfer (in bytes)
    \param rank the node rank whre to do the transfer
    \param vm_queue the queue to use for the transfer

    \return an int where 0 is success and -1 is operation failed

    This operation is one-sided and non-blocking.

  */
  int readDmaVM(const unsigned long localOffset,const unsigned long remOffset,
		const int size,const unsigned int rank,const unsigned int vm_queue);


  //! Write data to a remote node memory location
  /*!

    \param localOffset the offset on the local node where the data to be written is located
    \param remOffset the offset on the remote node where the data should written to
    \param size the size of the transfer (in bytes)
    \param rank the node rank from where to write the data
    \param vm_queue the queue to use for the transfer

    \return an int where 0 is success and -1 is operation failed

    This operation is one-sided and non-blocking.

  */
  int writeDmaVM(const unsigned long localOffset,const unsigned long remOffset,
		 const int size,const unsigned int rank,const unsigned int vm_queue);



  //! Send data to a remote node
  /*!

    \param localOffset the local offset where the data to be sent is located
    \param size the size of the transfer
    \param rank the node rank where to send the data
    \param vm_queue the queue to use for the transfer
    \warning
    \return an int where 0 is success and -1 is operation failed

    This operation is non-blocking. It is a two-sided operation which
    means that should be a matching recvDmaVM on the remote node. To
    make sure the operation is done (not hanging on the respective
    queue) a waitDmaVM needs to be called on the queue.
  */

  int sendDmaVM(const unsigned long localOffset,const int size,
		const unsigned int rank,const unsigned int vm_queue);

  //! Receive data from a remote node
  /*!

    \param localOffset the local offset where the received data should be placed
    \param size the size of the data to be received
    \param rank the rank from where to receive
    \param vm_queue the queue to use for the transfer
    \warning the queue number must match the respective sendDmaVM queue
    \return an int where 0 is success and -1 is operation failed

    This operation is a blocking operation: it will block until the data
    is received therefore do not call waitDmaVM for it. There should be
    a correspoding sendDmaVM from the remote node.
  */

  int recvDmaVM(const unsigned long localOffset,const int size,
		const unsigned int rank,const unsigned int vm_queue);

  //! Wait for all operations on a queue to finish
  /*!

    \param vm_queue the queue number to wait on
    \warning Operation not fully thread-safe (read below)
    \return  an int with the number of completed queue events or -1 on error

    WaitDmaVM makes sure that all operations (read, write, send) posted
    to a queue are finished. The operation is not fully thread-safe:
    when a thread enters a waitDmaVM, it will wait for all operations
    (including other threads' requests). A thread that posted requests
    on a queue and gets a return value from waitDmaVM of 0 (zero) only
    knows that some other thread might be waiting or waited for all
    requests.
  */
  int waitDmaVM(const unsigned int vm_queue);
  int waitDma2VM(const unsigned int vm_queue);

  /*@}*/

  //! \name Passive Communication
  /*@{*/
  //! Send data to a remote node using the passive channel
  /*!

    \param localOffset the local offset where to be sent is located
    \param size the size of the transfer
    \param rank the node rank where send the data
    \return an int where 0 is success and -1 is operation failed

    This operation uses the passive channel and therefore there's no
    queue for it. It is a non-blocking operation and it is two-sided
    which means that should be a matching recvDmaPassiveVM on the
    remote node. To make sure the operation is done a waitDmaPassiveVM
    needs to be called.
  */

  int sendDmaPassiveVM(const unsigned long localOffset,const int size,const unsigned int rank);


  //! Wait for all passive sends to finish
  /*!

    \warning Operation not fully thread-safe (read below)
    \return  an int with the number of completed queue events or -1 on error

    WaitDmaPassiveVM makes sure that all send passive operations are
    finished. The operation is not fully thread-safe: when a thread
    enters a waitDmaPassiveVM, it will wait for all operations
    (including other threads' requests). A thread that posted send
    passive requests and gets a return value from waitDmaPassiveVM of 0
    (zero) only knows that some other thread might be waiting or waited
    for all requests.

  */
  int waitDmaPassiveVM();

  //! Receive data using the passive channel
  /*!

    \param localOffset the local offset where to put the received data
    \param size the size of the data to be received
    \param senderRank the rank of the node that sent the data or -1 if the sender could not be established
    \warning this is a blocking function. Do not call waitDmaPassiveVM for it.
    \warning Not thread-safe.
    \return an int where 0 is success and -1 is operation failed

    receiveDmaPassiveVM will block and passively wait (no
    busy-looping) for any incoming data (there is no node
    parameter). There is no need to call waitDmaPassiveVM after this
    call. The function is not thread-safe and it is advised that only
    a single thread has the function to do passive receives.
  */

  int recvDmaPassiveVM(const unsigned long localOffset, const int size, int *senderRank);

  /*@}*/

  //! \name Commands
  /*@{*/
  //! Get a command from master node
  /*!

    \warning this operation is for worker nodes only (all nodes with rank > 0)

    \return an int with the command number or -1 on error

    A command is simply a number that can be used to trigger different
    actions on the worker node. This is a blocking operation: it will
    block until the master node has sent the respective command.
  */
  int  getCommandVM(void);

  //! Set (or send) a command to the worker nodes
  /*!

    \param cmd the command number
    \warning this operation is for the master node only

    \return an int where 0 is success and -1 is operation failed

    A command is simply a number that can be used to trigger different
    actions on the worker node.  This operation will block until all
    worker nodes have done the respective getCommandVM.
  */
  int  setCommandVM(const int cmd);//only master,(rank 0)


  //! Get a command from a particular node
  /*!

    \param rank the rank number to get the command from
    \warning no localhost support
    \return a long with the command number

    A command is simply a number that can be used to trigger different
    actions on the node. This is a blocking operation: it will block
    until the remote node (cannot be itself) has sent the respective
    command.

  */
  long getCommandFromNodeIdVM(const unsigned int rank);

  //! Set (or send) a command to a particular node
  /*!

    \param rank the node rank where to send the command
    \param cmd the command number to send
    \warning no localhost support

    \return a long where 0 is success and -1 is operation failed

    This operation will block until the node (rank) has done the
    respective getCommandFromNodeIdVM.
  */
  long setCommandToNodeIdVM(const unsigned int rank,const long cmd);

  /*@}*/

  //! \name Tile Counter
  /*@{*/
  //! Atomic fetch-and-add of the tile counter
  /*!

    \param val the value to add

    \return an unsigned long with the previous (old) value of the tile counter

    The atomic fetch-and-add operation will atomically add the val
    argument to the current value of the counter. The old value is
    returned (fetched).
  */

  unsigned long atomicFetchAddTileCntVM(const unsigned long val);


  //! Atomic compare-and-swap of the tile counter
  /*!

    \param cmpVal the compare value
    \param swapVal the value to be swapped

    \return an unsigned long with the previous (old) value of the tile counter

    The atomic compare-and-swap operation will atomically compare the
    counter value with the argument cmpVal and in case they are equal
    the counter value will be replaced with the swapVal argument. The
    previous (old) value of the counter is returned.

  */
  unsigned long atomicCmpSwapTileCntVM(const unsigned long cmpVal,const unsigned long swapVal);

  //! Reset the tile counter (make it zero)
  /*!

    \return an int where 0 is success and -1 is operation failed
  */
  int atomicResetTileCntVM(void);

  /*@}*/


  //! \name Atomic Counters
  /*@{*/
  //! Atomic fetch-and-add a global atomic counter
  /*!

    \param val the value to be added
    \param vm_counter the counter number where to do the operation
    \warning avoid busy-looping (e.g. while(1) ) on a counter to wait for a particular value

    \return and unsigned long with the previous (old) value of the global atomic counter

    A global atomic counter is a counter (value) that is globally
    accessible (from all nodes). The operations allowed are
    atomic. The atomic fetch-and-add operation will atomically add the
    val argument to the current value of the counter. The old value is
    returned (fetched).

  */
  unsigned long atomicFetchAddCntVM(const unsigned long val,const unsigned int vm_counter);


  //! Atomic compare-and-swap a global atomic counter
  /*!

    \param cmpVal the compare value
    \param swapVal the value to be swapped
    \param vm_counter the counter number where to do the operation
    \warning avoid busy-looping (e.g. while(1) ) on a counter to wait for a particular value

    \return an unsigned long with the previous (old) value of the atomic counter

    The atomic compare-and-swap operation will atomically compare the
    counter value with the argument cmpVal and in case they are equal
    the counter value will be replaced with the swapVal argument. The
    previous (old) value of the counter is returned.

  */
  unsigned long atomicCmpSwapCntVM(const unsigned long cmpVal,const unsigned long swapVal,const unsigned int vm_counter);

  //! Reset a global atomic counter (make it zero)
  /*!

    \param vm_counter the counter number to be reset-ed

    \return an int where 0 is success and -1 is operation failed
  */
  int atomicResetCntVM(const unsigned int vm_counter);

  /*@}*/

  //! \name Global Lock
  /*@{*/
  //! Lock the global resource lock
  /*!

    \return an int where 0 is success (got lock) and -1 is operation failed (did not got lock)

    The global resource lock is a lock (synchronization mechanism)
    that works globally. All nodes can use it to limit access to a
    shared resource using the lock-unlock semantics. Once a node got
    the lock it can be sure it is the only one. Since it is a global
    resource is should be used wisely and used in a relaxed manner
    (try not to busy-loop to get the lock).
  */
  int globalResourceLockVM(void);

  //! Unlock the global resource lock
  /*!

    \return an int where 0 is success and -1 is operation failed (not owner of the lock)

    Unlock the previously locked global resource lock. For this
    operation to be successful, the node has to own lock.
  */
  int globalResourceUnlockVM(void);

  /*@}*/

  //! \name Communication errors
  /*@{*/
  //! Check for errors on a communication queue
  /*!

    \param vm_queue The queue to check for errors

    \return a byte vector that has # node elements. On each position of the vector: 1=error,0=nonerror
  */
  unsigned char *getErrorVectorVM(const unsigned int vm_queue);

  /*@}*/


  //! \name Number of events
  /*@{*/

  //! Get number of open DMA requests on a queue
  /*!
    \param vm_queue the queue number to check

    \return an int with the number of open requests or -1 on error

    A queue has a limit on the number of outstanding requests
    (currently 1024) that can be posted to it without entering an
    error state. A waitDmaVM must be done before this limit to make
    sure requests are finished. This function returns the number of
    requests open that is, not processed by a waitDmaVM.
  */
  int openDMARequestsVM(const unsigned int vm_queue);


  //! Get number of open passive DMA requests
  /*!

    \return an int with the number of open requests or -1 on error

    The passive channel has a limit on the number of outstanding
    requests (currently 1024) that can be posted without calling a
    waitDmaPassiveVM. This function returns the number of
    requests open that is, not processed by a waitDmaPassiveVM.

  */
  int openDMAPassiveRequestsVM();

  //fds of event channels
  int getChannelFdVM(void);

  /*@}*/

#ifdef __cplusplus
}
#endif

#endif
