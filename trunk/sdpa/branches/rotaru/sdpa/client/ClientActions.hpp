#ifndef SDPA_CLIENT_ACTIONS_HPP
#define SDPA_CLIENT_ACTIONS_HPP 1

namespace sdpa { namespace client {
  class ClientActions {
  public:
    virtual void configure() = 0;
    virtual void config_ok() = 0;
    virtual void config_nok() = 0;

    virtual void submit() = 0;
    virtual void cancel() = 0;
    virtual void shutdown() = 0;
  };
}}

#endif
