#include "error_handler.hpp"

static void DefaultErrorHandler (void)
{
#ifndef NDEBUG
  exit (1);
#else
  // ignore
#endif
}

static fhg::log::ErrorHandler fhg_error_handler (&DefaultErrorHandler);

namespace fhg
{
  namespace log
  {
    void error_handler ()
    {
      fhg_error_handler ();
    }

    void install_error_handler (ErrorHandler h)
    {
      fhg_error_handler = h;
    }
  }
}
