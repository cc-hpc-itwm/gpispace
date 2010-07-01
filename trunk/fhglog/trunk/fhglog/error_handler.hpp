#ifndef FHG_LOG_ERROR_HANDLER_HPP
#define FHG_LOG_ERROR_HANDLER_HPP 1

#include <stdlib.h>

namespace fhg
{
  namespace log
  {
    typedef void (*ErrorHandler)(void);
    extern void error_handler ();
    extern void install_error_handler (ErrorHandler);
  }
}

#endif
