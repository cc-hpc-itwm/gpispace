#ifndef FHGLOG_CONFIGURATION_HPP
#define FHGLOG_CONFIGURATION_HPP 1

#include <fhglog/fhglog-config.hpp>
#include <fhglog/DefaultConfiguration.hpp>

#include <iostream>

namespace fhg { namespace log {
  inline
  void configure ()
  {
    DefaultConfiguration()();
  }

  inline
  void configure (int ac, char *av[])
  {
    // parameters currently ignored
    configure();
  }
}}

#endif
