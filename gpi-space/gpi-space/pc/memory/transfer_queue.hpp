#ifndef GPI_SPACE_PC_MEMORY_TRANSFER_QUEUE_HPP
#define GPI_SPACE_PC_MEMORY_TRANSFER_QUEUE_HPP

#include <set>

#include <boost/shared_ptr.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/condition_variable.hpp>

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
        transfer_queue_t (const std::size_t id);
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
        void enable ();
        bool is_disabled () const;

        typedef boost::mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::condition_variable condition_type;
        typedef boost::shared_ptr<boost::thread> thread_ptr;

        void worker ();

        mutable mutex_type m_mutex;
        mutable condition_type m_resume_condition;
        std::size_t m_id;
        bool m_enabled;

        task_queue_t m_task_queue;
        thread_ptr m_thread;

        task_set_t m_dispatched;
        // dispatched list -> contains dispatched tasks
        // finished list   -> contains finished tasks
      };
    }
  }
}

#endif
