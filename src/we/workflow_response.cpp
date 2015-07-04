#include <we/workflow_response.hpp>

#include <we/type/value/peek_or_die.hpp>
#include <we/type/value/serialize.hpp>

#include <rpc/client.hpp>

namespace we
{
  void workflow_response ( pnet::type::value::value_type const& rpc_server
                         , pnet::type::value::value_type const& value
                         )
  {
    fhg::rpc::remote_endpoint remote_client
      ( pnet::type::value::peek_or_die<std::string> (rpc_server, {"address"})
      , pnet::type::value::peek_or_die<unsigned int> (rpc_server, {"port"})
      );
    fhg::rpc::sync_remote_function<void (pnet::type::value::value_type)>
      (remote_client, "set_result") (value);
  }
}
