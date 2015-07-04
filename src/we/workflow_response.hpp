#pragma once

#include <we/type/value.hpp>

namespace we
{
  //! \note rpc_server fields: address :: string, port :: unsigned int
  void workflow_response
    ( pnet::type::value::value_type const& rpc_server
    , pnet::type::value::value_type const& value
    );
}
