#include "simple.hpp"
#include "cookie.hpp"

namespace gspc
{
  namespace net
  {
    namespace auth
    {
      simple_auth_t::simple_auth_t ()
        : m_cookie (gspc::net::auth::get_cookie ())
      {}

      simple_auth_t::simple_auth_t (std::string const &cookie)
        : m_cookie (cookie)
      {}

      bool simple_auth_t::is_authorized (std::string const &cookie) const
      {
        return m_cookie.empty() || m_cookie == cookie;
      }

      void simple_auth_t::set_cookie (std::string const &cookie)
      {
        m_cookie = cookie;
      }

      std::string const & simple_auth_t::get_cookie () const
      {
        return m_cookie;
      }
    }
  }
}
