#include <gpi-space/pc/memory/transfer_queue.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      transfer_queue_t::transfer_queue_t()
        : m_thread (&transfer_queue_t::worker, this)
      {}

      void
      transfer_queue_t::worker ()
      {
        for (;;)
        {
          m_task_queue.get()();
        }
      }

      void transfer_queue_t::enqueue (std::packaged_task<void()> task)
      {
        m_task_queue.put (std::move (task));
      }
    }
  }
}
