#ifndef GSPC_NET_SERVE_HPP
#define GSPC_NET_SERVE_HPP

#include <string>

#include <gspc/net/server_fwd.hpp>
#include <gspc/net/server/queue_manager_fwd.hpp>

#include <boost/asio/io_service.hpp>

namespace gspc
{
  namespace net
  {
    server_ptr_t serve ( std::string const& url
                       , boost::asio::io_service&
                       , server::queue_manager_t&
                       );

  }
}

#endif
