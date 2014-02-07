#include "counter.hpp"

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      counter_t::counter_t(const gpi::pc::type::size_t start)
        : m_counter (start)
      {}

      gpi::pc::type::size_t counter_t::value () const
      {
        lock_type lock (m_mutex);
        return m_counter;
      }

      gpi::pc::type::size_t counter_t::inc ()
      {
        lock_type lock (m_mutex);
        return ++m_counter;
      }

      void counter_t::reset (const gpi::pc::type::size_t val)
      {
        lock_type lock (m_mutex);
        m_counter = val;
      }
    }
  }
}
