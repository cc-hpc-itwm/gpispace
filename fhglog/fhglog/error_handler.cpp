#include <iostream>
#include <unistd.h>
#include <cstdio>
#include "error_handler.hpp"

static void DefaultErrorHandler (void)
{
  std::clog << "Program termination triggered by FhgLog..." << std::endl;
  fflush(NULL);
  _exit (42);
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
