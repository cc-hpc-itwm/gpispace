// alexander.petry@itwm.fraunhofer.de

#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhglog/format.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/measure_average_time.hpp>
#include <util-generic/testing/printer/chrono.hpp>

BOOST_AUTO_TEST_CASE (formatting_performance)
{
  BOOST_REQUIRE_LT
    ( fhg::util::testing::measure_average_time<std::chrono::microseconds>
      ( []()
        {
          format ( fhg::log::default_format::LONG()
                 , FHGLOG_MKEVENT_HERE (TRACE, "hello world!")
                 );
        }
       , 100000
      )
    , std::chrono::microseconds (10)
    );
}
