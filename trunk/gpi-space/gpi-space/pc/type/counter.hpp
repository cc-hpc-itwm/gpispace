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
        inline counter_t (const gpi::pc::type::size_t = 0);

        inline gpi::pc::type::size_t operator++ () { return inc(); }
        inline gpi::pc::type::size_t operator* () { return value(); }
        inline gpi::pc::type::size_t inc ();
        inline gpi::pc::type::size_t value () const;
        inline void move_to (const gpi::pc::type::size_t);
        inline void reset (gpi::pc::type::size_t = 0);
      private:
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        mutable mutex_type m_mutex;
        gpi::pc::type::size_t m_counter;
      };
    }
  }
}

#include "counter.ipp"

#endif
