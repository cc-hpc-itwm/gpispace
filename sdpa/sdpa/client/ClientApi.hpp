#ifndef SDPA_CLIENT_CLIENT_API_HPP
#define SDPA_CLIENT_CLIENT_API_HPP 1

#include <sdpa/memory.hpp>
#include <fhglog/fhglog.hpp>
#include <sdpa/client/Client.hpp>
#include <sdpa/job_states.hpp>
#include <boost/utility.hpp>
#include <sdpa/events/events.hpp>

#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace sdpa { namespace client {
  class ClientApi : boost::noncopyable {
  public:
    static config_t config()
    {
      return Client::config();
    }

    ClientApi (const config_t &cfg)
      : pimpl (Client::ptr_t (new Client(cfg, "sdpac-" + boost::uuids::to_string (boost::uuids::random_generator()()))))
    {
    }

    sdpa::status::code wait_for_terminal_state (job_id_t id, job_info_t& job_info)
    {
      return pimpl->wait_for_terminal_state (id, job_info);
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
