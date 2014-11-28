#define BOOST_TEST_MODULE fhglog_event
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>
#include "dump_event.common.hpp"

#include <fhg/util/now.hpp>

BOOST_AUTO_TEST_CASE (encode_decode)
{
  const std::string evts (gen_event().encoded());

  BOOST_REQUIRE_EQUAL (fhg::log::LogEvent::from_string (evts).encoded(), evts);
}
