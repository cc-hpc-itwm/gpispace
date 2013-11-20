#ifndef SDPA_CLIENT_CLIENT_API_HPP
#define SDPA_CLIENT_CLIENT_API_HPP

#include <sdpa/client/Client.hpp>


namespace sdpa
{
  namespace client
  {
    class ClientApi : Client
    {
    public:
      using Client::config;

      ClientApi (const config_t &cfg)
        : Client (cfg)
      {}

      using Client::wait_for_terminal_state;
      using Client::submitJob;
      using Client::cancelJob;
      using Client::queryJob;
      using Client::deleteJob;
      using Client::retrieveResults;
    };
  }
}

#endif
