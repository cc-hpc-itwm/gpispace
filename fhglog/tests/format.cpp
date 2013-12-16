// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE formatter
#include <boost/test/unit_test.hpp>

#include <sstream> // std::ostringstream
#include <fhglog/fhglog.hpp>
#include <fhglog/format.hpp>

BOOST_AUTO_TEST_CASE (short_severity)
{
#define CHECK(SEV, SEV_STR)                                             \
  BOOST_REQUIRE_EQUAL                                                   \
    (fhg::log::format ("%s", FHGLOG_MKEVENT_HERE (SEV, "")), SEV_STR)

  CHECK (TRACE, "T");
  CHECK (DEBUG, "D");
  CHECK (INFO, "I");
  CHECK (WARN, "W");
  CHECK (ERROR, "E");
  CHECK (FATAL, "F");

#undef CHECK
}

BOOST_AUTO_TEST_CASE (long_severity)
{
#define CHECK(SEV, SEV_STR)                                             \
  BOOST_REQUIRE_EQUAL                                                   \
    (fhg::log::format ("%S", FHGLOG_MKEVENT_HERE (SEV, "")), SEV_STR)

  CHECK (TRACE, "TRACE");
  CHECK (DEBUG, "DEBUG");
  CHECK (INFO, "INFO");
  CHECK (WARN, "WARN");
  CHECK (ERROR, "ERROR");
  CHECK (FATAL, "FATAL");

#undef CHECK
}

BOOST_AUTO_TEST_CASE (path)
{
  const fhg::log::LogEvent event ( fhg::log::LogLevel::DEBUG
                                 , "tests/test_formatter.cpp"
                                 , "main", __LINE__, "hello"
                                 );
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%P", event), "tests/test_formatter.cpp");
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%p", event), "test_formatter.cpp");
}

BOOST_AUTO_TEST_CASE (line)
{
  const fhg::log::LogEvent event ( fhg::log::LogLevel::DEBUG
                                 , "tests/test_formatter.cpp"
                                 , "main", 1002, "hello"
                                 );
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%L", event), "1002");
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%l", event), "");
}

BOOST_AUTO_TEST_CASE (message)
{
  BOOST_REQUIRE_EQUAL
    (fhg::log::format ("%m", FHGLOG_MKEVENT_HERE (DEBUG, "hello")), "hello");
}

BOOST_AUTO_TEST_CASE (newline)
{
  std::ostringstream ostr;
  ostr << std::endl;
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%n", fhg::log::LogEvent()), ostr.str());
}

BOOST_AUTO_TEST_CASE (percentage_escapes_percentage)
{
  BOOST_REQUIRE_EQUAL (fhg::log::format ("%%", fhg::log::LogEvent()), "%");
  BOOST_REQUIRE_EQUAL (fhg::log::format ("test %%", fhg::log::LogEvent()), "test %");
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
