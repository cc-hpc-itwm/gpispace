#ifndef GPI_SPACE_PC_MEMORY_TRANSFER_QUEUE_HPP
#define GPI_SPACE_PC_MEMORY_TRANSFER_QUEUE_HPP

#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>

#include <gpi-space/pc/memory/task_queue.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class transfer_queue_t
      {
      public:
        explicit
        transfer_queue_t();
        ~transfer_queue_t ();

        void enqueue (task_ptr const &);
        void enqueue (task_list_t const &);

        void disable ();

        // issue a "wait" on the queue
        //
        //    post-cond: all previously issued transfers are finished
        //
        //    return: the number of finished transfers
        //
        //     BEHOLD: the value returned  can be useless depending on the
        //             number of process  containers issueing wait(), this
        //             call might return the correct value, 0, or anything
        //             else.
        std::size_t wait ();

      private:
        bool is_disabled () const;

        typedef boost::shared_ptr<boost::thread> thread_ptr;

        void worker ();

        mutable boost::mutex m_mutex;
        bool m_enabled;

        task_queue_t m_task_queue;
        thread_ptr m_thread;

        mutable boost::mutex _mutex_dispatched;
        task_set_t m_dispatched;
        // dispatched list -> contains dispatched tasks
        // finished list   -> contains finished tasks
      };
    }
  }
}

#endif
