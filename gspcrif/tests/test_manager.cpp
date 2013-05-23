#define BOOST_TEST_MODULE GspcRifManagerTests
#include <boost/test/unit_test.hpp>

#include <errno.h>
#include <gspc/rif/manager.hpp>

struct F
{
  F ()
  {}
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (test_start_stop)
{
  static const int NUM_IERATIONS = 100;

  gspc::rif::manager_t manager;

  for (size_t iter = 0 ; iter < NUM_IERATIONS ; ++iter)
  {
    manager.start ();
    manager.stop ();
  }
}

BOOST_AUTO_TEST_SUITE_END()
