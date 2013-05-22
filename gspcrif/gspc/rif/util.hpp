#ifndef GSPC_RIF_UTIL_HPP
#define GSPC_RIF_UTIL_HPP

#include <gspc/rif/types.hpp>

namespace gspc
{
  namespace rif
  {
    int parse (std::string const &, argv_t &);
    int parse (const char *cmdline, argv_t &);
    int parse (const char *buffer, size_t len, argv_t &);
  }
}

#endif
