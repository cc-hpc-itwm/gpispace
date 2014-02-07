#ifndef GPI_SPACE_PC_TYPE_COUNTER_HPP
#define GPI_SPACE_PC_TYPE_COUNTER_HPP 1

#include <boost/thread.hpp>
#include <boost/noncopyable.hpp>

#include <gpi-space/pc/type/typedefs.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      struct counter_t : boost::noncopyable
      {
        explicit
        inline counter_t (const gpi::pc::type::size_t start = 0)
          : m_counter (start)
        {}

        operator gpi::pc::type::size_t () const
        {
          lock_type lock (m_mutex);
          return m_counter;
        }
        inline gpi::pc::type::size_t inc ()
        {
          lock_type lock (m_mutex);
          return ++m_counter;
        }
        inline void reset (gpi::pc::type::size_t val)
        {
          lock_type lock (m_mutex);
          m_counter = val;
        }
      private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        mutable mutex_type m_mutex;
        gpi::pc::type::size_t m_counter;
      };
    }
  }
}

#endif
