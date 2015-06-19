// mirko.rahn@itwm.fraunhofer.de

#include <drts/workflow_response.hpp>

#include <rpc/client.hpp>

#include <we/type/value/serialize.hpp>

#include <boost/thread/scoped_thread.hpp>

namespace gspc
{
  void workflow_response ( std::string const& trigger_address
                         , unsigned short const& trigger_port
                         , pnet::type::value::value_type const& value
                         )
  {
    fhg::rpc::remote_endpoint remote_client (trigger_address, trigger_port);
    fhg::rpc::sync_remote_function<void (pnet::type::value::value_type)>
      (remote_client, "set_result") (value);
  }
}
