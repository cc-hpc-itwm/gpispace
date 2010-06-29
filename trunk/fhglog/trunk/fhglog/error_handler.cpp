#include "error_handler.hpp"

static void DefaultErrorHandler (void)
{
#ifndef NDEBUG
  exit (1);
#else
  // ignore
#endif
}

fhg::log::ErrorHandler fhg::log::error_handler (&DefaultErrorHandler);

namespace fhg
{
  namespace log
  {
    ErrorHandler get_error_handler ()
    {
      return error_handler;
    }

    void set_error_handler (ErrorHandler h)
    {
      error_handler = h;
    }
  }
}
