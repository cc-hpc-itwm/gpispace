// bernd.loerwald@itwm.fraunhofer.de

#include <fhglog/fhglog.hpp>

namespace utils
{
  struct logger_with_minimum_log_level
  {
    logger_with_minimum_log_level()
      : log (fhg::log::getLogger())
    {
      log.setLevel (fhg::log::LogLevel::MIN_LEVEL);
    }
    ~logger_with_minimum_log_level()
    {
      log.removeAllAppenders();
    }

    fhg::log::logger_t log;
  };
}
