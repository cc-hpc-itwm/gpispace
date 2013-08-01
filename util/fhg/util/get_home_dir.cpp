#include <fhg/util/get_home_dir.hpp>

#include <string.h>             // memset
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>                // getpwuid_r

namespace fhg
{
  namespace util
  {
    std::string get_home_dir ()
    {
      int rc;
      struct passwd pwd;
      struct passwd *result;

      char buf [4096];

      memset (buf, 0, sizeof(buf));

      rc = getpwuid_r (getuid (), &pwd, buf, sizeof(buf), &result);
      if (not result || pwd.pw_dir == 0)
      {
        return "/";
      }
      else
      {
        return pwd.pw_dir;
      }
    }
  }
}
