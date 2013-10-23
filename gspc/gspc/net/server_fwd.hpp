#ifndef GSPC_NET_SERVER_FWD_HPP
#define GSPC_NET_SERVER_FWD_HPP

#include <boost/shared_ptr.hpp>

namespace gspc
{
  namespace net
  {
    class server_t;

    typedef boost::shared_ptr<server_t> server_ptr_t;
  }
}

#endif
