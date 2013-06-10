#ifndef GSPC_NET_CLIENT_FWD_HPP
#define GSPC_NET_CLIENT_FWD_HPP

#include <boost/shared_ptr.hpp>

namespace gspc
{
  namespace net
  {
    class client_t;

    typedef boost::shared_ptr<client_t> client_ptr_t;
  }
}

#endif
