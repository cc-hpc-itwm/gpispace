#ifndef GSPC_NET_COMMON_SAFE_RUN_IO_SERVICE_HPP
#define GSPC_NET_COMMON_SAFE_RUN_IO_SERVICE_HPP

#include <boost/asio.hpp>

namespace gspc
{
  namespace net
  {
    int safe_run_io_service (boost::asio::io_service *);
  }
}

#endif
