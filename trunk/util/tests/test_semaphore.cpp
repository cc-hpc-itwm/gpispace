#define BOOST_TEST_MODULE UtilSemaphoreTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/semaphore.hpp>

BOOST_AUTO_TEST_CASE ( dining_philosophers )
{
}

BOOST_AUTO_TEST_CASE ( enter_critical_section )
{
  fhg::util::thread::semaphore s (1);
  s.P();
  s.V();
}
