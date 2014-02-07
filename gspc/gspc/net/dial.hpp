#ifndef GSPC_NET_DIAL_HPP
#define GSPC_NET_DIAL_HPP

#include <string>

#include <gspc/net/client_fwd.hpp>
#include <boost/asio/io_service.hpp>

namespace gspc
{
  namespace net
  {
    client_ptr_t dial (std::string const &url, boost::asio::io_service&);
  }
}

#endif
