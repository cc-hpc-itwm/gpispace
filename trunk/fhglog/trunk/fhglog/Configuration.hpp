#ifndef FHGLOG_CONFIGURATION_HPP
#define FHGLOG_CONFIGURATION_HPP 1

#if defined(HAVE_CONFIG_H)
#  include <fhglog/fhglog-config.hpp>
#endif

#include <fhglog/fhglog.hpp>

#include <iostream>

namespace fhg { namespace log {
  class DefaultConfiguration {
    public:
      void operator() () throw() {
#if FHGLOG_DISABLE_LOGGING != 1
#ifndef NDEBUG
        std::clog << "I: performing default logging configuration" << std::endl;
#endif
        try {
          getLogger().removeAllAppenders();
          getLogger().addAppender(Appender::ptr_t(new StreamAppender("console")))->setFormat(Formatter::Default());
          getLogger().setLevel(LogLevel::TRACE);
        } catch (const std::exception& ex) {
          std::clog << "E: Could not configure the logging environment: " << ex.what() << std::endl;
        } catch (...) {
          std::clog << "E: Could not configure the logging environment: " << "unknown error" << std::endl;
        }
        std::clog.flush();
#endif
      }
  };

  class Configurator {
    public:
      static void configure() {
        configure(DefaultConfiguration());
      }
      static void configure(int , char **) {
        // parameters currently ignored
        configure();
      }
      template<class CF> static void configure(CF cf) {
        (cf)();
      }
  };
}}

#endif
