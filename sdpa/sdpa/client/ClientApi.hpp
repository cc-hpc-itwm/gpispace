#ifndef SDPA_CLIENT_CLIENT_API_HPP
#define SDPA_CLIENT_CLIENT_API_HPP

#include <sdpa/client/Client.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace sdpa
{
  namespace client
  {
    class ClientApi : boost::noncopyable, Client
    {
    public:
      using Client::config;

      ClientApi (const config_t &cfg)
        : Client (cfg, "sdpac-" + boost::uuids::to_string (boost::uuids::random_generator()()))
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
