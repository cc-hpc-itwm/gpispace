#include <fhglog/fhglog-config.hpp>
#include <iostream>
#include <unistd.h>
#include <cstdio>
#include "error_handler.hpp"

static void DefaultErrorHandler (void)
{
#if defined(FHGLOG_EXIT_ON_ERROR) && FHGLOG_EXIT_ON_ERROR == 1
  std::clog << "Program termination triggered by FhgLog..." << std::endl;
  fflush(NULL);
  _exit (FHGLOG_EXIT_ON_ERROR_CODE);
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
