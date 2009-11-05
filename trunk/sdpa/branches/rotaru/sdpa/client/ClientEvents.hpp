#ifndef SDPA_CLIENT_EVENTS_HPP
#define SDPA_CLIENT_EVENTS_HPP 1

#include <seda/IEvent.hpp>
#include <sdpa/client/types.hpp>

namespace sdpa { namespace client {
  class StartUp : public seda::IEvent
  {
    public:
      explicit
      StartUp(const config_t &a_config) : config_(a_config) {}
      std::string str() const { return "StartUp"; }

      const config_t &config() const { return config_; }
    private:
      config_t config_;
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
      explicit ConfigNOK(const std::string &a_reason)
        : reason_(a_reason)
      { }
      const std::string &reason() const { return reason_; }
      std::string str() const { return "ConfigNOK"; }
    private:
      std::string reason_;
  };
}}

#endif
