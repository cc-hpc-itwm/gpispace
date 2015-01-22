// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE formatter_performance
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhglog/format.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/now.hpp>

BOOST_AUTO_TEST_CASE (formatting_performance)
{
  double t (-fhg::util::now());

  for (std::size_t count (0); count < 100000; ++count)
  {
    format ( fhg::log::default_format::LONG()
           , FHGLOG_MKEVENT_HERE (TRACE, "hello world!")
           );
  }

  t += fhg::util::now();

  BOOST_REQUIRE_LT (t, 1.0);
}
