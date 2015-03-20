#define BOOST_TEST_MODULE fhglog_event
#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <fhglog/LogMacros.hpp>
#include <tests/dump_event.common.hpp>

BOOST_AUTO_TEST_CASE (encode_decode)
{
  const std::string evts (gen_event().encoded());

  BOOST_REQUIRE_EQUAL (fhg::log::LogEvent::from_string (evts).encoded(), evts);
}
