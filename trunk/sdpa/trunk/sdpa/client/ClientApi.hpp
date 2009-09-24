#ifndef SDPA_CLIENT_CLIENT_API_HPP
#define SDPA_CLIENT_CLIENT_API_HPP 1

#include <sdpa/memory.hpp>
#include <sdpa/client/Client.hpp>

namespace sdpa { namespace client {
  class ClientApi {
  public:
    typedef sdpa::shared_ptr<ClientApi> ptr_t;
    typedef std::string config_t;

    static ClientApi::ptr_t create(const Client::config_t &cfg
                                  ,const std::string &name_prefix="sdpa.apps.client"
                                  ,const std::string &output_stage="sdpa.apps.client.out")
    {
      ClientApi::ptr_t api(new ClientApi(Client::create(name_prefix, output_stage)));
      api->pimpl->start(cfg);
      return api;
    }

    ~ClientApi() {
      pimpl->shutdown();
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

    job_id_t submitJob(const job_desc_t &desc)
    {
      return pimpl->submitJob(desc);
    }

    void cancelJob(const job_id_t &jid)
    {
      return pimpl->cancelJob(jid);
    }

    int queryJob(const job_id_t &jid)
    {
      return pimpl->queryJob(jid);
    }

    void deleteJob(const job_id_t &jid)
    {
      return pimpl->deleteJob(jid);
    }

    Client::result_t retrieveResults(const job_id_t &jid)
    {
      return pimpl->retrieveResults(jid);
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
