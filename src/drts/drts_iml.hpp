// \todo Move to gspc/pnet/vmem.hpp.

#pragma once

#include <we/type/value.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

namespace gspc
{
  namespace pnet
  {
    namespace vmem
    {
      //! Convert an IML memory allocation \a handle to a petri net
      //! token usable for memory transfers.
      ::pnet::type::value::value_type handle_to_value
        (gpi::pc::type::handle_id_t handle);

      //! Convert an IML memory \a range to a petri net token usable for
      //! memory transfers.
      ::pnet::type::value::value_type range_to_value
        (gpi::pc::type::range_t range);

      //! Convert a stream slot information to a petri net token.
      ::pnet::type::value::value_type stream_slot_to_value
        ( gpi::pc::type::range_t const& metadata
        , gpi::pc::type::range_t const& data
        , char flag
        , std::size_t id
        );
    }
  }
}
