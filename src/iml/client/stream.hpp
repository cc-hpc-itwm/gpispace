#pragma once

#include <iml/client/iml.fwd.hpp>
#include <iml/client/iml.pimpl.hpp>
#include <iml/client/virtual_memory.fwd.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

#include <functional>
#include <string>
#include <utility>

namespace iml_client
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
    friend class scoped_iml_runtime_system;

    stream ( scoped_iml_runtime_system const&
           , std::string const& name
           , iml_client::vmem_allocation const& buffer
           , size_of_slot const&
           , std::function<void ( gpi::pc::type::range_t const metadata
                                , gpi::pc::type::range_t const data
                                , char const flag
                                , std::size_t const id
                                )
                          > on_slot_filled
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
