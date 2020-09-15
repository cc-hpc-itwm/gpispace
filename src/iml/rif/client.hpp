#pragma once

#include <iml/rif/entry_point.hpp>
#include <iml/rif/protocol.hpp>

#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/remote_function.hpp>

namespace fhg
{
  namespace iml
  {
    namespace rif
    {
      class client
      {
      public:
        client ( boost::asio::io_service& io_service
               , fhg::iml::rif::entry_point const& entry_point
               )
          : _endpoint (io_service, entry_point.hostname, entry_point.port)
          , kill (_endpoint)
          , start_vmem (_endpoint)
        {}

      private:
        fhg::rpc::remote_tcp_endpoint _endpoint;

      public:
        rpc::remote_function<protocol::kill> kill;
        rpc::remote_function<protocol::start_vmem> start_vmem;
      };
    }
  }
}
