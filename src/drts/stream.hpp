#pragma once

#include <drts/drts.fwd.hpp>
#include <drts/pimpl.hpp>
#include <drts/virtual_memory.fwd.hpp>

#include <we/type/value.hpp>

#include <functional>
#include <string>
#include <utility>

namespace gspc
{
  class stream
  {
  public:
    struct size_of_slot
    {
      explicit size_of_slot (std::size_t size)
        : _ (size)
        {}

      operator std::size_t const& () const
      {
        return _;
      }
    private:
      const std::size_t _;
    };
    struct number_of_slots
    {
      explicit number_of_slots (std::size_t count)
        : _ (count)
      {}

      operator std::size_t const& () const
      {
        return _;
      }
    private:
      const std::size_t _;
    };

  private:
    friend class scoped_runtime_system;

    stream ( scoped_runtime_system const&
           , gspc::scoped_vmem_segment_and_allocation const& buffer
           , size_of_slot const&
           , std::function<void (pnet::type::value::value_type const&)>
               on_slot_filled
           );
  public:
    void write (std::string const&);

    static void mark_free ( const char old_flag_value
                          , std::pair<void*, unsigned long> ptr_to_flag
                          );

    stream (stream const&) = delete;
    stream& operator= (stream const&) = delete;

    stream (stream&&);
    stream& operator= (stream&&) = delete;

    PIMPL (stream);
  };
}
