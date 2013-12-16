// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE every_n
#include <boost/test/unit_test.hpp>

#include <fhglog/fhglog.hpp>

#include <tests/utils.hpp>

#include <boost/preprocessor/repetition/repeat.hpp>

namespace
{
  template<typename T> T divru (T a, T b) { return (a + b - 1) / b; }
}

#define TEST_CASE_EVERY_N(IGNORE, N, iterations)                        \
BOOST_FIXTURE_TEST_CASE (every_ ## N, utils::logger_with_minimum_log_level) \
{                                                                       \
  std::size_t messages_logged (0);                                      \
  log.addAppender ( fhg::log::Appender::ptr_t                           \
                    (new utils::counting_appender (&messages_logged))   \
                  );                                                    \
                                                                        \
  for (size_t i (0); i < iterations; ++i)                               \
  {                                                                     \
    FHGLOG_DO_EVERY_N (N, LLOG (INFO, log, "iteration #" << i));        \
  }                                                                     \
                                                                        \
  BOOST_REQUIRE_EQUAL (messages_logged, divru (iterations, N));         \
}


BOOST_PP_REPEAT_FROM_TO (1, 51, TEST_CASE_EVERY_N, 50)
