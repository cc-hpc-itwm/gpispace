/* -*- mode: c++ -*- */

#include "counter.hpp"

#include <stdexcept>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      counter_t::counter_t()
        : m_counter (0)
      {}

      gpi::pc::type::size_t counter_t::inc ()
      {
        lock_type lock (m_mutex);
        gpi::pc::type::size_t new_count (m_counter + 1);
        if (! new_count) // TODO: branch hint unlikely
        {
          throw std::runtime_error ("cannot increment counter: would overflow");
        }
        m_counter = new_count;
        return new_count;
      }

      void counter_t::reset (const gpi::pc::type::size_t val)
      {
        lock_type lock (m_mutex);
        m_counter = val;
      }
    }
  }
}
