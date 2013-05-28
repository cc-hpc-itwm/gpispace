#ifndef GSPC_RIF_UTIL_HPP
#define GSPC_RIF_UTIL_HPP

#include <gspc/rif/types.hpp>

namespace gspc
{
  namespace rif
  {
    int parse (std::string const &, argv_t &, size_t & consumed);
    int parse (const char *buffer, size_t len, argv_t &, size_t & consumed);

    /**
       Return a unified exit code within the range [0,255].

       0-128 exit code by the process itself
       128+signal in case the process terminated due to a signal

       returns -EBUSY if it is not yet finished.
    */
    int make_exit_code (int status);
  }
}

#endif
