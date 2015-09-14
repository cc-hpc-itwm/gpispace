// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE formatter
#include <boost/test/unit_test.hpp>

#include <sstream> // std::ostringstream
#include <fhglog/LogMacros.hpp>
#include <fhglog/format.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

BOOST_AUTO_TEST_CASE (percentage_escapes_percentage)
{
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%%", fhg::log::LogEvent()), "%");
}

BOOST_AUTO_TEST_CASE (short_severity)
{
#define CHECK(SEV, SEV_STR)                                             \
  BOOST_REQUIRE_EQUAL                                                   \
    (fhg::log::format ("%s", FHGLOG_MKEVENT_HERE (SEV, "")), SEV_STR)

  CHECK (TRACE, "T");
  CHECK (INFO, "I");
  CHECK (WARN, "W");
  CHECK (ERROR, "E");

#undef CHECK
}

BOOST_AUTO_TEST_CASE (severity)
{
#define CHECK(SEV, SEV_STR)                                             \
  BOOST_REQUIRE_EQUAL                                                   \
    (fhg::log::format ("%S", FHGLOG_MKEVENT_HERE (SEV, "")), SEV_STR)

  CHECK (TRACE, "TRACE");
  CHECK (INFO, "INFO");
  CHECK (WARN, "WARN");
  CHECK (ERROR, "ERROR");

#undef CHECK
}

BOOST_AUTO_TEST_CASE (message)
{
  BOOST_REQUIRE_EQUAL
    (fhg::log::format ("%m", FHGLOG_MKEVENT_HERE (TRACE, "hello")), "hello");
}

//! \todo date
//! \todo tstamp
//! \todo tid
//! \todo pid
//! \todo tags

BOOST_AUTO_TEST_CASE (logger)
{
  //! \todo This should actually test something:
  //! put mutiple loggers in a chain, send event through
  const fhg::log::LogEvent event ( fhg::log::TRACE
                                 , "hello"
                                 );
}

BOOST_AUTO_TEST_CASE (newline)
{
  std::ostringstream ostr;
  ostr << std::endl;
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%n", fhg::log::LogEvent()), ostr.str());
}

BOOST_AUTO_TEST_CASE (format_flags_work_everywhere_in_string)
{
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%%", fhg::log::LogEvent()), "%");
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%%%%", fhg::log::LogEvent()), "%%");
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%%_%%", fhg::log::LogEvent()), "%_%");
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%% test", fhg::log::LogEvent()), "% test");
  BOOST_REQUIRE_EQUAL (fhg::log::format ("test %%", fhg::log::LogEvent()), "test %");
  BOOST_REQUIRE_EQUAL (fhg::log::format ("test %% test", fhg::log::LogEvent()), "test % test");
}

BOOST_AUTO_TEST_CASE (throw_on_invalid_escaped_sequence)
{
  BOOST_REQUIRE_THROW (fhg::log::check_format ("%U"), std::runtime_error);
  BOOST_REQUIRE_THROW (fhg::log::check_format ("%"), std::runtime_error);
  BOOST_REQUIRE_THROW (fhg::log::check_format ("test %-"), std::runtime_error);
}

//! \todo This does not test
BOOST_AUTO_TEST_CASE (NOTEST_use_short_format)
{
  fhg::log::format (fhg::log::default_format::SHORT(), fhg::log::LogEvent());
}

//! \todo This should test all format flags!
