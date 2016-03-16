#include <boost/test/unit_test.hpp>

#include <fhglog/event.hpp>
#include <fhglog/level.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

//! \todo This should test way more
BOOST_AUTO_TEST_CASE (event_ctor_should_regard_severity)
{
  const fhg::log::LogEvent evt
    (fhg::log::TRACE, "hello world!");
  BOOST_REQUIRE_EQUAL (evt.severity(), fhg::log::TRACE);
}

//! \todo This does not really test anything
BOOST_AUTO_TEST_CASE (ctor_should_not_throw_and_should_be_copyable)
{
  const fhg::log::LogEvent evt1
    (fhg::log::TRACE, "hello world!");
  const fhg::log::LogEvent evt2 (evt1);
  const fhg::log::LogEvent evt3
    (fhg::log::INFO, "blah!");
}
