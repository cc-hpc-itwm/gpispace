#ifndef GSPC_RIF_UTIL_HPP
#define GSPC_RIF_UTIL_HPP

#include <gspc/rif/types.hpp>

namespace gspc
{
  namespace rif
  {
    int parse (std::string const &, argv_t &, size_t & consumed);
    int parse (const char *buffer, size_t len, argv_t &, size_t & consumed);
  }
}

#endif
