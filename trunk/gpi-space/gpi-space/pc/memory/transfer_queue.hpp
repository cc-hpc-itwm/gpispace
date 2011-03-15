#ifndef GPI_SPACE_PC_MEMORY_TRANSFER_QUEUE_HPP
#define GPI_SPACE_PC_MEMORY_TRANSFER_QUEUE_HPP

#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>

#include <gpi-space/pc/memory/memory_transfer_t.hpp>
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
        transfer_queue_t (task_queue_t *async);
        ~transfer_queue_t ();

        void enqueue (memory_transfer_t const &);

        // request to pause the queue
        //   no new requests will be accepted
        //   there might still be requests queued,
        //   use flush to remove them
        void pause ();

        // unpause the queue
        void resume ();

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

        // wait until all queues are empty
        void flush ();

        bool is_paused () const;
      private:
        typedef boost::mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::condition_variable condition_type;
        typedef boost::shared_ptr<boost::thread> thread_ptr;
        typedef boost::shared_ptr<task_t> task_ptr;

        void worker ();
        void wait_until_unpaused () const;

        static void do_memcpy (memory_transfer_t mt);

        mutable mutex_type m_mutex;
        mutable condition_type m_resume_condition;
        bool m_paused;
        task_queue_t & m_blocking_tasks;
        task_queue_t m_task_queue;
        thread_ptr m_thread;

        std::set<task_ptr> m_dispatched;
        std::set<task_ptr> m_finished;
        // dispatched list -> contains dispatched tasks
        // finished list   -> contains finished tasks
      };
    }
  }
}

#endif
