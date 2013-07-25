#include "cookie.hpp"

#include <fstream>

#include <string.h>             // memset
#include <unistd.h>             // getuid
#include <stdlib.h>             // getenv
#include <sys/types.h>          // uid_t
#include <sys/stat.h>           // stat
#include <pwd.h>                // getpwuid_r
#include <fcntl.h>              // O_RDONLY

namespace gspc
{
  namespace net
  {
    namespace auth
    {
      static bool get_cookie_from_env ( std::string const &dflt
                                      , std::string const &name
                                      , std::string & out
                                      )
      {
        char *cookie = 0;

        if ((cookie = getenv (name.c_str ())))
        {
          out = cookie;
          return true;
        }
        else
        {
          out = dflt;
          return false;
        }
      }

      static bool get_cookie_from_home ( std::string const &dflt
                                       , std::string const &fname
                                       , std::string & out
                                       )
      {
        int rc;
        struct passwd pwd;
        struct passwd *result;
        struct stat stat_buf;
        char buf [4096];
        char cookie [4096];

        memset (&buf[0], 0, sizeof(buf));
        memset (&cookie[0], 0, sizeof(cookie));

        out = dflt;

        rc = getpwuid_r (getuid (), &pwd, buf, sizeof(buf), &result);
        if (not result)
        {
          return false;
        }

        std::string f (pwd.pw_dir);
        f += "/" + fname;

        int fd = open (f.c_str (), O_RDONLY);
        if (fd < 0)
        {
          return false;
        }

        rc = fstat (fd, &stat_buf);
        if (rc < 0)
        {
          close (fd);
          return false;
        }

        if (not S_ISREG (stat_buf.st_mode))
        {
          close (fd);
          return false;
        }

        if ( stat_buf.st_mode & S_IRWXG
           || stat_buf.st_mode & S_IRWXO
           )
        {
          close (fd);
          return false;
        }

        rc = read (fd, cookie, sizeof(cookie));
        if (rc < 0)
          return false;
        while (rc > 0)
        {
          if (cookie[rc] == '\n' || cookie [rc] == 0)
            cookie[rc] = 0;
          else
            break;
          --rc;
        }

        close (fd);
        out = cookie;

        return true;
      }

      static std::string get_default_cookie (std::string const &dflt)
      {
        std::string cookie;
        if (get_cookie_from_env (dflt, "GSPC_COOKIE", cookie))
          return cookie;
        if (get_cookie_from_home (dflt, ".gspc.cookie", cookie))
          return cookie;
        return dflt;
      }

      std::string const & get_cookie ()
      {
        static std::string cookie (get_default_cookie (""));
        return cookie;
      }
    }
  }
}
