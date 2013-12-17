// bernd.loerwald@itwm.fraunhofer.de

#include <fhglog/fhglog.hpp>

#include <fhg/util/counter.hpp>

#include <boost/format.hpp>

namespace utils
{
  namespace
  {
    fhg::util::counter<unsigned int> _logger_counter;
  }

  struct logger_with_minimum_log_level
  {
    logger_with_minimum_log_level()
      : log ( fhg::log::getLogger
              ((boost::format ("logger-%1%") % _logger_counter.next()).str())
            )
    {
      log.setLevel (fhg::log::LogLevel::MIN_LEVEL);
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
