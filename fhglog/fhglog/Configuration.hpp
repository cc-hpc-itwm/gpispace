#ifndef FHGLOG_CONFIGURATION_HPP
#define FHGLOG_CONFIGURATION_HPP 1

#include <fhglog/fhglog-config.hpp>
#include <fhglog/DefaultConfiguration.hpp>

#include <iostream>

namespace fhg { namespace log {
  class Configurator {
    public:
      static void configure() {
        DefaultConfiguration()();
      }
      static void configure(int , char *[]) {
        // parameters currently ignored
        configure();
      }
  };

  inline
  void configure ()
  {
    Configurator::configure();
  }

  inline
  void configure (int ac, char *av[])
  {
    Configurator::configure(ac, av);
  }
}}

#endif
