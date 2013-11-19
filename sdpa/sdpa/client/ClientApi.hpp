#ifndef SDPA_CLIENT_CLIENT_API_HPP
#define SDPA_CLIENT_CLIENT_API_HPP 1

#include <sdpa/memory.hpp>
#include <fhglog/fhglog.hpp>
#include <sdpa/client/Client.hpp>
#include <sdpa/job_states.hpp>
#include <boost/utility.hpp>
#include <sdpa/events/events.hpp>

namespace sdpa { namespace client {
  class ClientApi : boost::noncopyable {
  public:
    static config_t config()
    {
      return Client::config();
    }

    ClientApi ( const config_t &cfg
              , const std::string &name_prefix="sdpac"
              , const std::string &output_stage="sdpa.apps.client.out"
              )
      : pimpl (Client::create(cfg, name_prefix, output_stage))
    {
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

    sdpa::status::code wait_for_terminal_state (job_id_t id, job_info_t& job_info)
    {
      job_id_list_t listJobIds;
      listJobIds.push_back (id);
      pimpl->subscribe (listJobIds);

      seda::IEvent::Ptr reply (pimpl->wait_for_reply (-1));

      if ( sdpa::events::JobFinishedEvent* evt
         = dynamic_cast<sdpa::events::JobFinishedEvent*> (reply.get())
         )
      {
        if (evt->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        return sdpa::status::FINISHED;
      }
      else if ( sdpa::events::JobFailedEvent* evt
              = dynamic_cast<sdpa::events::JobFailedEvent*> (reply.get())
              )
      {
        if (evt->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        job_info.error_code = evt->error_code();
        job_info.error_message = evt->error_message();
        return sdpa::status::FAILED;
      }
      else if ( sdpa::events::CancelJobAckEvent* evt
              = dynamic_cast<sdpa::events::CancelJobAckEvent*> (reply.get())
              )
      {
        if (evt->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        return sdpa::status::CANCELED;
      }
      else if ( sdpa::events::ErrorEvent *err
              = dynamic_cast<sdpa::events::ErrorEvent*>(reply.get())
              )
      {
        throw std::runtime_error
          ( "got error event: reason := "
          + err->reason()
          + " code := "
          + boost::lexical_cast<std::string>(err->error_code())
          );
      }
      else
      {
        throw std::runtime_error
          (std::string ("unexpected reply: ") + (reply ? reply->str() : "null"));
      }
    }

    job_id_t submitJob(const job_desc_t &desc) throw (ClientException)
    {
      return pimpl->submitJob(desc);
    }

    void cancelJob(const job_id_t &jid) throw (ClientException)
    {
      return pimpl->cancelJob(jid);
    }

    status::code queryJob(const job_id_t &jid) throw (ClientException)
    {
      return pimpl->queryJob(jid);
    }

    status::code queryJob(const job_id_t &jid, job_info_t & status)
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

  private:

    Client::ptr_t pimpl;
  };
}}

#endif
