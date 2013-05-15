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

      simple_auth_t::~simple_auth_t ()
      {}

      bool simple_auth_t::is_authorized (std::string const &cookie) const
      {
        if (m_cookie.empty ())
          return true;

        if (m_cookie == cookie)
          return true;

        return false;
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
