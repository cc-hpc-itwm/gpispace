#ifndef GSPC_RIF_UTIL_HPP
#define GSPC_RIF_UTIL_HPP

#include <vector>
#include <string>

namespace gspc
{
  namespace rif
  {
    int parse ( const char *cmdline
              , std::vector<std::string> &
              );
    int parse ( const char *buffer
              , size_t len
              , std::vector<std::string> &
              );
  }
}

#endif
