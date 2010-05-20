#ifndef FVM_PC_UTIL_HPP
#define FVM_PC_UTIL_HPP 1

#include <cstring>
#include <unistd.h>

#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <fvm-pc/assert.hpp>

namespace fvm
{
  namespace util {

    template < fvmAllocHandle_t (*Alloc)(fvmSize_t)
             , int (*Delete)(fvmAllocHandle_t)
             >
    struct generic_allocation
    {
      explicit generic_allocation ( fvmSize_t alloc_size
                                  , fvmAllocHandle_t hdl = 0
                                  )
        : handle(hdl)
        , size(alloc_size)
        , committed(false)
      {
        if (hdl == 0)
        {
          handle = Alloc(size);
          DLOG(DEBUG, "allocated " << size << " bytes");
        }
      }

      ~generic_allocation()
      {
        if (!committed && handle)
        {
          Delete(handle); handle = 0;
        }
      }

      /* marks the memory as commited, i.e. will not be freed in destructor */
      void commit()
      {
        committed = true;
        DLOG(DEBUG, "committed allocation: " << handle);
      }

      operator fvmAllocHandle_t () { return handle; }

      fvmAllocHandle_t handle;
      fvmSize_t size;
      bool committed;
    };

    typedef generic_allocation<&fvmGlobalAlloc, &fvmGlobalFree> global_allocation;
    typedef generic_allocation<&fvmLocalAlloc, &fvmLocalFree> local_allocation;

    inline void wait_for_communication(fvmCommHandle_t hdl, std::size_t numRetries = 3, unsigned int usec = 100000)
    {
      fvmCommHandleState_t comm_state;
      for (std::size_t i(0); i < numRetries; ++i)
      {
        comm_state = waitComm(hdl);
        if (comm_state == COMM_HANDLE_NOT_FINISHED)
        {
          DLOG(TRACE, "communication " << hdl << " not finished yet, retrying");
          usleep(usec);
        }
        else break;
      }
      if (comm_state != COMM_HANDLE_OK)
      {
        LOG(FATAL, "communication failed: hdl=" << hdl << " state=" << comm_state);
        // FIXME: throw specific exceptions here
        throw std::runtime_error("waitComm failed!");
      }
      DLOG(TRACE, "communication " << hdl << " finished");
    }

    /****************************************************************************************
     * The following functions are mainly used for "global" configuration stuff,
     * i.e. a structure that must be available on every node and keeps various
     * information about the algorithm's parameters.
     *
     *   Basic usage:
     *      1. one node initializes an array of configuration structures for each node
     *      2. distribute to all nodes
     *      N. each node gets its own structure
     *
     * It is always assumed, that the global allocation is of size:
     *    sizeof (Type).
     ****************************************************************************************/

    /*
     * puts one data element to a global allocation space
     *     data - pointer to the data element
     *   global - global allocation handle
     *     rank - to which node to put the data (by default the calling node will be used)
     *
     * warning: it uses the shared memory pointer to transfer data
     */
    template <typename Type> void put_data(const Type *data, fvmAllocHandle_t global, int rank = -1)
    {
      if (rank < 0) rank = fvmGetRank();

      const fvmSize_t transfer_size = sizeof(Type);
      assert(transfer_size <= fvmGetShmemSize());

      // copy to shared mem
      memcpy(fvmGetShmemPtr(), data, sizeof(Type));

      local_allocation scratch(transfer_size);
      ASSERT_LALLOC(scratch);

      DLOG(DEBUG, "putting data of node " << rank);
      fvmCommHandle_t comm_handle = fvmPutGlobalData(global, rank*sizeof(Type), sizeof(Type), 0, scratch);
      wait_for_communication(comm_handle);
    }

    /*
     * gets one data element from a global allocation
     *     data - where the retrieved shall be placed
     *   global - global allocation handle
     *     rank - from which node to get the data (by default the calling node will be used)
     *
     * warning: it uses the shared memory pointer to transfer data
     */
    template <typename Type> void get_data(Type *data, fvmAllocHandle_t global, int rank = -1)
    {
      if (rank < 0) rank = fvmGetRank();

      local_allocation scratch(sizeof(Type));
      ASSERT_LALLOC(scratch);

      DLOG(DEBUG, "getting data from node " << rank);
      fvmCommHandle_t comm_handle = fvmGetGlobalData(global, rank*sizeof(Type), sizeof(Type), 0, scratch);
      wait_for_communication(comm_handle);

      // copy from shared mem
      memcpy(data, fvmGetShmemPtr(), sizeof(Type));
    }

    /*
     * warning: it uses the shared memory pointer to transfer data
     *
     * Distributes a single data item to all nodes.
     *
     *	  data   - pointer to the data item
     *    global - global allocation handle to distribute to
     *
     * Copies data to the global-allocation space of all nodes.
     *
     */
    template <typename Type> void distribute_single_data(const Type *data, fvmAllocHandle_t global)
    {
      const fvmSize_t transfer_size = sizeof(Type);
      assert(transfer_size <= fvmGetShmemSize());

      // copy to shared mem
      DLOG(DEBUG, "copying " << transfer_size << " bytes to shmem: " << fvmGetShmemPtr());
      memcpy(fvmGetShmemPtr(), data, transfer_size);

      local_allocation scratch(transfer_size);
      ASSERT_LALLOC(scratch);
      for (size_t node(0); node < fvmGetNodeCount(); ++node)
      {
        DLOG(DEBUG, "distributing data to node: " << node);
        fvmCommHandle_t comm_handle = fvmPutGlobalData(global, node * transfer_size, transfer_size, 0, scratch);
        wait_for_communication(comm_handle);
      }
    }

    /*
     * Distributes a data array pointing to getNodeCount() data items to all
     * nodes (each gets it's own element)
     *
     * [d0,d1,d2] ---> to global allocation
     *	  d0 -> n0
     *	  d1 -> n1
     *      d2 -> n2
     */
    template <typename Type> void distribute_data(const Type *data, fvmAllocHandle_t global)
    {
      const fvmSize_t transfer_size = fvmGetNodeCount() * sizeof(Type);
      assert(transfer_size <= fvmGetShmemSize());

      // copy to shared mem
      DLOG(DEBUG, "copying " << transfer_size << " bytes to shmem: " << fvmGetShmemPtr());
      memcpy(fvmGetShmemPtr(), data, transfer_size);

      local_allocation scratch(transfer_size);
      ASSERT_LALLOC(scratch);

      DLOG(DEBUG, "distributing data to all nodes");
      fvmCommHandle_t comm_handle = fvmPutGlobalData(global, 0, transfer_size, 0, scratch);
      wait_for_communication(comm_handle);
    }
  }
}

#endif
