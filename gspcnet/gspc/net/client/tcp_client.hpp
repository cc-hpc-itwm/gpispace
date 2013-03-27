#ifndef GSPC_NET_CLIENT_TCP_CLIENT_HPP
#define GSPC_NET_CLIENT_TCP_CLIENT_HPP

#include <string>

#include <boost/system/error_code.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <gspc/net/client/base_client.hpp>

namespace gspc
{
  namespace net
  {
    namespace client
    {
      typedef gspc::net::client::base_client<boost::asio::ip::tcp> tcp_client;
    }
  }
}

#endif
