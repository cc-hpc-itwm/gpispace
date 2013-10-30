#ifndef GPI_SPACE_PC_MEMORY_TRANSFER_MANAGER_HPP
#define GPI_SPACE_PC_MEMORY_TRANSFER_MANAGER_HPP 1

#include <boost/thread/thread.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/memory/memory_transfer_t.hpp>
#include <gpi-space/pc/memory/thread_pool.hpp>
#include <gpi-space/pc/memory/transfer_queue.hpp>
#include <gpi-space/pc/memory/task_queue.hpp>

#include <gpi-space/pc/memory/memory_buffer.hpp>
#include <gpi-space/pc/memory/memory_buffer_pool.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
        // enqueue:
        //
        //  2 kinds of transfers
        //
        //       a) local  (memcpy)
        //       b) remote (GPI/RDMA)
        //
        //  bool t::is_local ():
        //      return src->is_local (t) && dst->is_local(t)
        //                 shm: return true
        //                 gpi: return true iff is_local(hdl)
        //                             || (handle+offset+amount) is on this node
        //
        //  copy-function:
        //      SRC -> DST
        //
        //      SHM -> local -> memcpy (GPI.local or SHM)
        //      SHM -> other -> throw  (user has to do: SHM->GPI->GPI)
        //
        //      GPI.local  -> SHM        -> memcpy
        //      GPI.remote -> SHM        -> throw
        //      GPI.local  -> GPI.local  -> memcpy
        //      GPI.local  -> GPI.remote -> writeDMA (multiple)
        //      GPI.remote -> GPI.local  -> readDMA  (multiple)
        //      GPI.remote -> GPI.remote -> throw
        //
        //      fun/0 = get_transfer_function (src, dst, queue)
        //            = src.get_transfer_function (dst, queue)
        //
        //          memcpy:
        //              return boost::bind ( ::memcpy
        //                                 , dst.ptr<char>(dst_offset)
        //                                 , src.ptr<char>(src_offset)
        //                                 , amount
        //                                 )
        //          writeDMA:
        //              return boost::bind ( api::write_dma
        //                                 , ref(api)
        //                                 , params, ...
        //                                 )
        //          readDMA:
        //              return boost::bind ( api::read_dma
        //                                 , ref(api)
        //                                 , params, ...
        //                                 )
        //
        //    -> different transfer tasks
        //
        // m_queues.at(queue).enqueue (t);
        //    m_tasks.push(new transfer_task (create_work(t)...,*this));
        //
        // tasks:
        //    wait:
        //       foreach w in async:
        //           w->wait()
        //           move w to finished
        //           call signal
        //    transfer
        //        if work->is_async
        //           if work->would_block
        //              tasks::wait (...).execute();
        //           assert (!work->would_block)
        //           async.push (work)
        //           work->execute()
        //
        // work:
        //    bool is_async ()
        //        SHM: returns always false
        //        GPI: return always true
        //
        //    bool would_block ()
        //        SHM: return always false
        //        GPI: if #active_dma_transfers >= queue_depth
        //
        //    execute ()
        //        memcpy
        //           m_queue.finished (this);
        //        readdma/writedma
        //
        //    wait ()
        //        perform a wait operation
        //        -> waitOnQueue (queue)
        //        -> or nop
        //
        // queue:
        //    1 worker thread      // one per queue
        //    shared pool of mules // "memcpy" threads
        //
        // queue worker
        //    task* = m_queue.pop()
        //    task->execute ()
        //
        //    running->push (work);
        //    if work->would_block()
        //      pool->push (work);
        //    else
        //      async->push (work);
        //      work->execute(); // readdma writedma
        //                       // might still block -> wait_intern()
        //
        // copy_worker_pool
        //    work->execute
        //
        // wait () // executed by process container
        //    send wait request to queue worker
        //    wait until async and running are empty
        //    clear finished
        //    return old finished size

      class transfer_manager_t
      {
      public:
        static const size_t DEF_BUFFER_SIZE = 4194304;

        void start ( const std::size_t number_of_queues
                   , const std::size_t memcpy_pool_size = 2
                   );

        void transfer (memory_transfer_t const &);
        std::size_t wait_on_queue (const std::size_t queue);

        std::size_t num_queues () const { return m_queues.size (); }
      private:
        typedef boost::shared_ptr<transfer_queue_t> queue_ptr;
        typedef std::vector<queue_ptr> transfer_queues_t;
        typedef boost::shared_ptr<task_t> task_ptr;
        typedef buffer_pool_t<buffer_t> memory_pool_t;

        void worker();

        transfer_queues_t m_queues;
        task_queue_t m_worker_queue;
        thread_pool_t m_worker_pool;
        memory_pool_t m_memory_buffer_pool;
      };
    }
  }
}

#endif // GPI_SPACE_PC_MEMORY_TRANSFER_MANAGER_HPP
