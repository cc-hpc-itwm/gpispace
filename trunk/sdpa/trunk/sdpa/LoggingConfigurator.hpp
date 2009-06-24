#ifndef SDPA_LOGGINGCONFIGURATOR_HPP
#define SDPA_LOGGINGCONFIGURATOR_HPP 1

#include <iostream>
#include <sdpa/common.hpp>
#include <sdpa/logging.hpp>

#if SDPA_HAVE_LOG4CPP
#include <log4cpp/Category.hh>
#include <log4cpp/BasicConfigurator.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#endif

namespace sdpa { namespace logging {
  class DefaultConfiguration {
    public:
      void operator() () throw() {
#if SDPA_ENABLE_LOGGING == 1
#if SDPA_HAVE_LOG4CPP == 1
        std::clog << "I: performing default logging configuration for log4cpp" << std::endl;
        try {
          ::log4cpp::BasicConfigurator::configure();
          const ::log4cpp::AppenderSet appenders(::log4cpp::Category::getRoot().getAllAppenders());
          for (::log4cpp::AppenderSet::const_iterator it(appenders.begin()); it != appenders.end(); it++) {
            ::log4cpp::PatternLayout *pl(new ::log4cpp::PatternLayout());
            pl->setConversionPattern("%R %p %c %x - %m%n");
            (*it)->setLayout(pl);
          }
          ::log4cpp::Category::setRootPriority(::log4cpp::Priority::DEBUG);
        } catch (const std::exception& ex) {
          std::clog << "E: Could not configure the logging environment: " << ex.what() << std::endl;
        } catch (...) {
          std::clog << "E: Could not configure the logging environment: " << "unknown error" << std::endl;
        }
        std::clog.flush();
#endif
#endif
      }
  };

  class Configurator {
    public:
      static void configure() {
        configure(DefaultConfiguration());
      }
      template<class CF> static void configure(CF cf) {
        (cf)();
      }
  };
}
}

#endif // ! SDPA_LOGGINGCONFIGURATOR_HPP
