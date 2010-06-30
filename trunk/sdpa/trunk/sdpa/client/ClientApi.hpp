#ifndef SDPA_CLIENT_CLIENT_API_HPP
#define SDPA_CLIENT_CLIENT_API_HPP 1

#include <sdpa/memory.hpp>
#include <fhglog/fhglog.hpp>
#include <sdpa/client/Client.hpp>

namespace sdpa { namespace client {
  class ClientApi {
  public:
    typedef sdpa::shared_ptr<ClientApi> ptr_t;

    static config_t config()
    {
      return Client::config();
    }

    static ClientApi::ptr_t create(const config_t &cfg
                                  ,const std::string &name_prefix="sdpa.apps.client"
                                  ,const std::string &output_stage="sdpa.apps.client.out") throw (ClientException)
    {
      ClientApi::ptr_t api(new ClientApi(Client::create(name_prefix, output_stage)));
      api->pimpl->start(cfg);
      return api;
    }

    ~ClientApi() {
      try
      {
        pimpl->shutdown();
      }
      catch (std::exception &ex)
      {
        LOG(FATAL, "client->shutdown() failed!" << ex.what());
      }
      catch (...)
      {
        LOG(FATAL, "client->shutdown() failed due to an unknown reason!");
      }
    }

    const std::string &version() const
    {
      return pimpl->version();
    }

    const std::string &copyright() const
    {
      return pimpl->copyright();
    }

    const std::string &contact() const
    {
      return pimpl->contact();
    }

    const std::string &build_timestamp() const
    {
      return pimpl->build_timestamp();
    }

    job_id_t submitJob(const job_desc_t &desc) throw (ClientException)
    {
      return pimpl->submitJob(desc);
    }

    void cancelJob(const job_id_t &jid) throw (ClientException)
    {
      return pimpl->cancelJob(jid);
    }

    std::string queryJob(const job_id_t &jid) throw (ClientException)
    {
      return pimpl->queryJob(jid);
    }

    void deleteJob(const job_id_t &jid) throw (ClientException)
    {
      return pimpl->deleteJob(jid);
    }

    result_t retrieveResults(const job_id_t &jid) throw (ClientException)
    {
      return pimpl->retrieveResults(jid);
    }

    const std::string &input_stage() const
    {
      return pimpl->input_stage();
    }

    const std::string &output_stage() const
    {
      return pimpl->output_stage();
    }

    void configure_network(const config_t &config)
    {
      pimpl->action_configure_network(config);
    }

    void shutdown_network()
    {
      pimpl->action_shutdown_network();
    }
  private:
    ClientApi(const Client::ptr_t &c)
      : pimpl(c)
    {
    }

    ClientApi(const ClientApi&);
    const ClientApi& operator=(const ClientApi&);

    Client::ptr_t pimpl;
  };
}}

#endif
