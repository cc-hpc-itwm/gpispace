#pragma once

#include <fhg/util/thread/queue.hpp>

#include <boost/thread/scoped_thread.hpp>

#include <future>

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

        void enqueue (std::packaged_task<void()>);

      private:
        void worker ();

        fhg::thread::queue<std::packaged_task<void()>> m_task_queue;
        boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
          m_thread;
      };
    }
  }
}
