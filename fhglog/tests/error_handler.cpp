#include <fhglog/fhglog.hpp>

#define BOOST_TEST_MODULE error_handler
#include <boost/test/unit_test.hpp>

namespace
{
  bool error_flag (false);

  void handle_error()
  {
    error_flag = true;
  }
}

BOOST_AUTO_TEST_CASE (error_handler_should_be_called_on_fatal)
{
  fhg::log::install_error_handler (&handle_error);

  LOG (FATAL, "testing error handler");

  BOOST_REQUIRE (error_flag);
}
