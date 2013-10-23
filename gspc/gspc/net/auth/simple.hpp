#ifndef GSPC_NET_AUTH_SERVICE_HPP
#define GSPC_NET_AUTH_SERVICE_HPP

#include <gspc/net/auth.hpp>

namespace gspc
{
  namespace net
  {
    namespace auth
    {
      class simple_auth_t : public gspc::net::auth_t
      {
      public:
        simple_auth_t ();
        explicit
        simple_auth_t (std::string const &);
        ~simple_auth_t ();

        bool is_authorized (std::string const &cookie) const;

        void set_cookie (std::string const &);
        std::string const & get_cookie () const;
      private:
        std::string m_cookie;
      };
    }
  }
}

#endif
