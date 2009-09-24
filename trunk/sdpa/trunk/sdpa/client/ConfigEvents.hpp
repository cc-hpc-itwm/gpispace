#ifndef SDPA_CLIENT_CONFIG_EVENTS_HPP
#define SDPA_CLIENT_CONFIG_EVENTS_HPP 1

#include <seda/IEvent.hpp>

namespace sdpa { namespace client {
  class StartUp : public seda::IEvent
  {
    public:
      explicit
      StartUp(const std::string &config) : config_(config) {}
      std::string str() const { return "StartUp"; }

      const std::string &config() const { return config_; }
    private:
      std::string config_;
  };

  class Shutdown : public seda::IEvent
  {
    public:
      std::string str() const { return "Shutdown"; }
  };
  class ShutdownComplete : public seda::IEvent
  {
    public:
      std::string str() const { return "ShutdownComplete"; }
  };

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
