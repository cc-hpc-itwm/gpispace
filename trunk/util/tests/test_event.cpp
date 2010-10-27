#define BOOST_TEST_MODULE UtilThreadEventTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/thread/event.hpp>

BOOST_AUTO_TEST_CASE ( signal_event )
{
  using namespace fhg::util::thread;
  typedef event<int> my_event;

  my_event e;
  e.notify (42);

  int answer (0);
  e.wait (answer);

  BOOST_CHECK_EQUAL (42, answer);
}
