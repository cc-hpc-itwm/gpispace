#include "simple.hpp"

#include <fstream>

#include <unistd.h>             // getuid
#include <stdlib.h>             // getenv
#include <sys/types.h>          // uid_t
#include <pwd.h>                // getpwuid_r

namespace gspc
{
  namespace net
  {
    namespace auth
    {
      static std::string get_or_create_cookie (std::string const &dflt)
      {
        char *cookie = 0;
        int rc = 0;
        char buf [4096];

        if ((cookie = getenv ("GSPC_COOKIE")))
        {
          return cookie;
        }

        struct passwd pwd;
        struct passwd *result;
        rc = getpwuid_r (getuid (), &pwd, buf, sizeof(buf), &result);
        if (result)
        {
          std::string fname (pwd.pw_dir);
          fname += "/.gspc.cookie";

          std::ifstream ifs (fname.c_str ());
          if (ifs)
          {
            std::string cookie_str;
            ifs >> std::noskipws >> cookie_str;

            return cookie_str;
          }
        }

        return dflt;
      }

      simple_auth_t::simple_auth_t ()
        : m_cookie (get_or_create_cookie (""))
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
