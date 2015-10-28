#pragma once

#include <boost/thread/thread.hpp>

#include <gpi-space/pc/type/typedefs.hpp>
#include <gpi-space/pc/memory/memory_transfer_t.hpp>
#include <gpi-space/pc/memory/transfer_queue.hpp>

#include <gpi-space/pc/memory/memory_buffer.hpp>

#include <fhg/util/thread/queue.hpp>

namespace gpi
{
  namespace api
  {
    class gpi_api_t;
  }
  namespace pc
  {
    namespace memory
    {
      class transfer_manager_t
      {
      public:
        transfer_manager_t (api::gpi_api_t&);

        type::memcpy_id_t transfer (memory_transfer_t const&);
        void wait (type::memcpy_id_t const&);

        std::size_t num_queues () const { return m_queues.size (); }
      private:
        typedef boost::shared_ptr<transfer_queue_t> queue_ptr;
        typedef std::vector<queue_ptr> transfer_queues_t;
        typedef boost::shared_ptr<task_t> task_ptr;

        std::size_t _next_memcpy_id;
        std::map<std::size_t, boost::shared_ptr<task_t>> _task_by_id;

        transfer_queues_t m_queues;
        fhg::thread::ptr_queue<buffer_t> m_memory_buffer_pool;
      };
    }
  }
}
