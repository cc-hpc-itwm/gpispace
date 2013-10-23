#ifndef GSPC_NET_SERVER_TCP_SERVER_HPP
#define GSPC_NET_SERVER_TCP_SERVER_HPP

#include <string>

#include <boost/system/error_code.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <gspc/net/server/base_server.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      typedef gspc::net::server::base_server<boost::asio::ip::tcp> tcp_server;
    }
  }
}

#endif
