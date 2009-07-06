#ifndef SDPA_CLIENT_CONFIG_EVENTS_HPP
#define SDPA_CLIENT_CONFIG_EVENTS_HPP 1

#include <seda/IEvent.hpp>

namespace sdpa { namespace client {
  class ConfigOK : public seda::IEvent
  {
    public:
      std::string str() const { return "ConfigOK"; }
  };

  class ConfigNOK : public seda::IEvent
  {
    public:
      std::string str() const { return "ConfigNOK"; }
  };
}}

#endif
