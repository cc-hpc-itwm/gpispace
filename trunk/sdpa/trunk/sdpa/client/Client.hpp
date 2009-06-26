#ifndef SDPA_CLIENT_HPP
#define SDPA_CLIENT_HPP 1

#include <string>

#include <seda/Strategy.hpp>
#include <sdpa/client/ClientActions.hpp>

namespace sdpa { namespace client {
  class Client : public ClientActions {
  public:
    Client() {}
    ~Client() {}

    void configure() {}
    void config_ok() {}
    void config_nok() {}

    void submit() {}
    void cancel() {}
    void shutdown() {}
  };
}}

#endif
