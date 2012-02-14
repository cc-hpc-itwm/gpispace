#include <fhglog/logger_config.hpp>
#include <fhglog/StreamAppender.hpp>
#include <iostream>

struct wrapped_sink
{
  template <typename A>
  wrapped_sink(A a)
    : appender(a)
  {}

  void operator() (const fhg::log::LogEvent & e) const
  {
    appender->append(e);
  }

  fhg::log::Appender::ptr_t appender;
};

int main()
{
  int errcount (0);

  using namespace fhg::log;

  logger_config config;

  {
    std::cerr << std::boolalpha;
    std::cerr << "enabled(DEBUG) := "
              << config.enabled (LogLevel::DEBUG)
              << std::endl;
  }
  {
    std::cerr << std::boolalpha;
    std::cerr << "enabled(DEF_LEVEL) := "
              << config.enabled (LogLevel::DEF_LEVEL)
              << std::endl;
  }

  std::stringstream sstr;
  wrapped_sink wrapped_appender(new StreamAppender("stream", sstr, "%m"));
  config.sink (wrapped_appender);
  config.sink()(LogEvent ());

  return errcount;
}
