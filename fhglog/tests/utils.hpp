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

  class counting_appender : public fhg::log::Appender
  {
  public:
    counting_appender (std::size_t* counter)
      : fhg::log::Appender ("counting_appender")
      , _counter (counter)
    {}

    void append (const fhg::log::LogEvent &evt)
    {
      ++(*_counter);
    }

    void flush () {}
  private:
    std::size_t* _counter;
  };
}
