#ifndef SDPA_CLIENT_CLIENT_API_HPP
#define SDPA_CLIENT_CLIENT_API_HPP 1

#include <sdpa/memory.hpp>
#include <fhglog/fhglog.hpp>
#include <sdpa/client/Client.hpp>

#include <boost/utility.hpp>

namespace sdpa { namespace client {
  class ClientApi : boost::noncopyable {
  public:
    typedef sdpa::shared_ptr<ClientApi> ptr_t;

    static config_t config()
    {
      return Client::config();
    }

    static ClientApi::ptr_t create( const config_t &cfg
                                  , const std::string &name_prefix="sdpac"
                                  , const std::string &output_stage="sdpa.apps.client.out") throw (ClientException)
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
        LOG(ERROR, "client->shutdown() failed!" << ex.what());
      }
      catch (...)
      {
        LOG(ERROR, "client->shutdown() failed due to an unknown reason!");
      }
    }

    void subscribe(const job_id_t& jobId)
        {
        job_id_list_t listJobIds;
        listJobIds.push_back(jobId);
        pimpl->subscribe(listJobIds);
        }

    void subscribe(const job_id_list_t& listJobIds)
        {
        pimpl->subscribe(listJobIds);
        }

    seda::IEvent::Ptr  waitForNotification(const sdpa::client::Client::timeout_t& t = -1)
        {
                return pimpl->wait_for_reply(t);
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

    int queryJob(const job_id_t &jid, job_info_t & status)
    {
      return pimpl->queryJob(jid, status);
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

    Client::ptr_t pimpl;
  };
}}

#endif
