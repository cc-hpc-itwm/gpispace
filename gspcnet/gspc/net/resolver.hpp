#ifndef GSPC_NET_RESOLVER_HPP
#define GSPC_NET_RESOLVER_HPP

#include <string>
#include <boost/system/error_code.hpp>

namespace gspc
{
  namespace net
  {
    template <class Proto>
    struct resolver
    {
      typedef typename Proto::endpoint endpoint_type;

      static
      endpoint_type resolve ( std::string const &address
                            , boost::system::error_code &ec
                            );
    };
  }
}

#endif
