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
      template <typename Buffer>
      class buffer_pool_t
      {
      public:
        typedef Buffer buffer_type;

        buffer_pool_t ();
        ~buffer_pool_t ();

        void add (buffer_type *);
        void del (buffer_type **);

        buffer_type *acquire ();
        void release (buffer_type *);
      private:
        typedef std::list<buffer_type*> buffer_list_t;

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
