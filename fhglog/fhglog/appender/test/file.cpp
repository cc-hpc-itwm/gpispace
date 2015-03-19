// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE file_appender
#include <boost/test/unit_test.hpp>

#include <fhglog/LogMacros.hpp>
#include <fhglog/appender/file.hpp>
#include <fhglog/format.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/read_file.hpp>
#include <fhg/util/temporary_file.hpp>

BOOST_AUTO_TEST_CASE (throw_on_unwritable_file)
{
  BOOST_REQUIRE_THROW
    (fhg::log::FileAppender ( "/non-existing-dir/test.log"
                            ,  fhg::log::default_format::LONG()
                            ).append (fhg::log::LogEvent())
    , std::exception
    );
}

BOOST_AUTO_TEST_CASE (remaining)
{
  fhg::util::temporary_file const _ ("test_file_appender.cpp.log");

  fhg::log::FileAppender appender ("test_file_appender.cpp.log", "%m");

  appender.append (FHGLOG_MKEVENT_HERE (ERROR, "hello world!"));
  appender.flush();

  BOOST_REQUIRE_EQUAL
    (fhg::util::read_file ("test_file_appender.cpp.log"), "hello world!");
}
