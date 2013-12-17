// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE once
#include <boost/test/unit_test.hpp>

#include <fhglog/fhglog.hpp>

#include <tests/utils.hpp>

BOOST_FIXTURE_TEST_CASE (only_log_once_in_50, utils::logger_with_minimum_log_level)
{
  const std::size_t N (50);

  std::size_t messages_logged (0);
  log.addAppender ( fhg::log::Appender::ptr_t
                    (new utils::counting_appender (&messages_logged))
                  );

  for (size_t i (0); i < N; ++i)
  {
    FHGLOG_DO_ONCE (LLOG (INFO, log, "iteration #" << i));
  }

  BOOST_REQUIRE_EQUAL (messages_logged, 1);
}
