#ifndef GSPC_NET_SERVE_HPP
#define GSPC_NET_SERVE_HPP

#include <gspc/net/server_fwd.hpp>
#include <gspc/net/server/queue_manager_fwd.hpp>

#include <boost/system/error_code.hpp>

namespace gspc
{
  namespace net
  {
    server_ptr_t serve ( const char *url
                       , server::queue_manager_t &
                       );
    server_ptr_t serve ( const char *url
                       , server::queue_manager_t &
                       , boost::system::error_code & ec
                       );
  }
}

#endif
