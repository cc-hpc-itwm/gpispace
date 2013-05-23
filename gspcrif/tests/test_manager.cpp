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
  gspc::rif::manager_t manager;
  manager.start ();

  sleep (1);

  manager.stop ();
}

BOOST_AUTO_TEST_SUITE_END()
