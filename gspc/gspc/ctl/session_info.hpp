#ifndef GSPC_CTL_SESSION_INFO_HPP
#define GSPC_CTL_SESSION_INFO_HPP

#include <string>

namespace gspc
{
  namespace ctl
  {
    struct session_info_t
    {
      session_info_t ()
        : pid (0)
        , name ()
        , puburl ()
        , path ()
        , started (0)
      {}

      int         pid;
      std::string name;
      std::string puburl;
      std::string path;
      long        started;
    };
  }
}

#endif
