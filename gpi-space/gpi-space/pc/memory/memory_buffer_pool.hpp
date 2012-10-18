#ifndef GPI_SPACE_PC_MEMORY_BUFFER_POOL_HPP
#define GPI_SPACE_PC_MEMORY_BUFFER_POOL_HPP

#include <unistd.h>

#include <list>
#include <boost/thread.hpp>

namespace gpi
{
  namespace pc
  {
    namespace memory
    {
      class buffer_t;

      class buffer_pool_t
      {
      public:
        static const size_t DEF_BUFFER_SIZE = 4194304;

        buffer_pool_t ();
        ~buffer_pool_t ();

        void add_buffer (size_t sz = DEF_BUFFER_SIZE);

        buffer_t *acquire ();
        void release (buffer_t *);
      private:
        typedef std::list<buffer_t*> buffer_list_t;

        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;
        typedef boost::condition_variable_any condition_type;

        mutable mutex_type m_mutex;
        mutable condition_type m_buffer_available;

        buffer_list_t m_buffers;

        size_t m_min_size;
        size_t m_max_size;
        size_t m_acquire_counter;
        size_t m_release_counter;
      };
    }
  }
}

#endif
