#ifndef FHGLOG_CONFIGURATION_HPP
#define FHGLOG_CONFIGURATION_HPP 1

#include <fhglog/fhglog-config.hpp>
#include <fhglog/DefaultConfiguration.hpp>

#include <iostream>

namespace fhg { namespace log {
  class Configurator {
    public:
      static void configure() {
        configure(DefaultConfiguration());
      }
      static void configure(int , char *[]) {
        // parameters currently ignored
        configure();
      }
      template<class CF> static void configure(CF cf) {
        (cf)();
      }
  };

  template <typename Fun>
  inline
  void configure (Fun fun)
  {
    fun();
  }

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
