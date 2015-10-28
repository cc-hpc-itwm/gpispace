#pragma once

#include <fhg/util/thread/queue.hpp>

#include <gpi-space/pc/memory/task.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread/scoped_thread.hpp>

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

        void enqueue (task_ptr const &);

      private:
        void worker ();

        fhg::thread::queue<boost::shared_ptr<task_t>>  m_task_queue;
        boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
          m_thread;
      };
    }
  }
}
