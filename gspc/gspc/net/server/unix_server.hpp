#ifndef GSPC_NET_SERVER_UNIX_SERVER_HPP
#define GSPC_NET_SERVER_UNIX_SERVER_HPP

#include <string>

#include <boost/system/error_code.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include <gspc/net/server/base_server.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      typedef gspc::net::server::base_server<boost::asio::local::stream_protocol> unix_server;

      boost::system::error_code resolve_address ( std::string const &
                                                , unix_server::endpoint_type &
                                                );
    }
  }
}

#endif
