#ifndef GSPC_NET_CLIENT_UNIX_CLIENT_HPP
#define GSPC_NET_CLIENT_UNIX_CLIENT_HPP

#include <string>

#include <boost/system/error_code.hpp>
#include <boost/asio/local/stream_protocol.hpp>

#include <gspc/net/client/base_client.hpp>

namespace gspc
{
  namespace net
  {
    namespace client
    {
      typedef gspc::net::client::base_client<boost::asio::local::stream_protocol> unix_client;
    }
  }
}

#endif
