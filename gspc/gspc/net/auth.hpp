#ifndef GSPC_NET_AUTH_HPP
#define GSPC_NET_AUTH_HPP

#include <string>

namespace gspc
{
  namespace net
  {
    class auth_t
    {
    public:
      virtual ~auth_t () {}

      virtual bool is_authorized (std::string const &cookie) const = 0;
      virtual std::string const & get_cookie () const = 0;
    };
  }
}

#endif
