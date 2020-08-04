#include <gspc/installation_path.hpp>

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_CASE (our_installation_does_not_throw)
{
  BOOST_CHECK_NO_THROW (gspc::installation_path{});
}
