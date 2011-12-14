#include <fhglog/fhglog.hpp>

static bool error_flag (false);

static void handle_error ()
{
  error_flag = true;
}

int main()
{
  fhg::log::install_error_handler ( &handle_error );

  LOG(FATAL, "testing error handler");

  if (error_flag)
    return EXIT_SUCCESS;
  else
    return EXIT_FAILURE;
}
