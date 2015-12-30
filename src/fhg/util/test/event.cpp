#define BOOST_TEST_MODULE UtilThreadEventTest
#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/thread/event.hpp>

BOOST_AUTO_TEST_CASE ( signal_event )
{
  using namespace fhg::util::thread;
  typedef event<int> my_event;

  my_event e;

  e.notify (41);
  BOOST_REQUIRE_EQUAL (41, e.wait());
}
