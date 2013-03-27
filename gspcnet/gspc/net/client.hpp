#ifndef GSPC_NET_CLIENT_HPP
#define GSPC_NET_CLIENT_HPP

#include <string>

namespace gspc
{
  namespace net
  {
    class client_t
    {
    public:
      virtual ~client_t () {}

      virtual int start () = 0;
      virtual int stop () = 0;
    };
  }
}

#endif
