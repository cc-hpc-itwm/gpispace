#include <fhglog/fhglog.hpp>

#define BOOST_TEST_MODULE error_handler
#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (exception_on_fatal)
{
  std::string const msg ("testing FATAL throws");

  try
  {
    LOG (FATAL, msg);
  }
  catch (std::runtime_error const& e)
  {
    BOOST_REQUIRE_EQUAL (e.what(), msg);
  }
}
