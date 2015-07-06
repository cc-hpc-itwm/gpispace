#pragma once

#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <string>

namespace we
{
  void workflow_response
    ( pnet::type::value::value_type const& rpc_server
    , pnet::type::value::value_type const& value
    );

  bool is_rpc_server_description
    (pnet::type::signature::signature_type const&);

  static inline std::string rpc_server_description_requirements()
  {
    return "the two fields 'address :: string' and 'port :: unsigned int'";
  }
}
