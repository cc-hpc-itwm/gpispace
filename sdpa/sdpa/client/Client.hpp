#ifndef SDPA_CLIENT_HPP
#define SDPA_CLIENT_HPP 1

#include <string>
#include <cstdlib>

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread.hpp> // condition variables

#include <seda/Stage.hpp>
#include <seda/Strategy.hpp>

#if defined(HAVE_CONFIG_H)
#include <sdpa/sdpa-config.hpp>
#endif

#include <sdpa/memory.hpp>
#include <sdpa/common.hpp>
#include <sdpa/types.hpp>

#include <sdpa/client/types.hpp>
#include <sdpa/client/exceptions.hpp>
#include <sdpa/client/job_info.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/job_states.hpp>

#include <sdpa/client/generated/ClientFsm_sm.h>

#include <fhg/revision.hpp>
#include <fhg/util/thread/queue.hpp>

namespace sdpa { namespace client {
  class Client : public seda::Strategy {
  public:
    typedef sdpa::shared_ptr<Client> ptr_t;
    typedef fhg::thread::queue<seda::IEvent::Ptr, std::list> event_queue_t;

    ~Client();

    // seda strategy
    void perform(const seda::IEvent::Ptr &);

    // API
    static Client::ptr_t create( const config_t &cfg
                               , const std::string &name_prefix="sdpac"
                              , const std::string &output_stage="sdpac-out");

    static config_t config()
    {
      using namespace sdpa::util;
      config_t cfg("client", "SDPAC_");
      std::string home(std::getenv("HOME"));
      cfg.specific_opts().add_options()
        ("orchestrator", po::value<std::string>()->default_value("orchestrator"),
         "name of the orchestrator")
        ("location", po::value<std::string>()->default_value("0.0.0.0:0"),
         "location of the client")
        ("name", po::value<std::string>()->default_value("sdpac"),
         "name of the client")
        ("config,C", po::value<std::string>()->default_value(home + "/.sdpa/configs/sdpac.rc"),
         "path to the configuration file")
        ;
      return cfg;
    }

    void start(const config_t &cfg) throw(ClientException);
    void shutdown() throw(ClientException);
    void subscribe(const job_id_list_t&) throw (ClientException);

    job_id_t submitJob(const job_desc_t &) throw (ClientException);
    void cancelJob(const job_id_t &) throw (ClientException);
    status::code queryJob(const job_id_t &) throw (ClientException);
    status::code queryJob(const job_id_t &, job_info_t &);
    void deleteJob(const job_id_t &) throw (ClientException);
    result_t retrieveResults(const job_id_t &) throw (ClientException);

    // Action implementations
    void action_shutdown();

    void forward_to_output_stage (const seda::IEvent::Ptr&) const;

    void action_store_reply(const seda::IEvent::Ptr &);

    typedef unsigned long long timeout_t;
    seda::IEvent::Ptr wait_for_reply() throw (Timedout);
    seda::IEvent::Ptr wait_for_reply(timeout_t t) throw (Timedout);

  private:
        Client(const std::string &a_name, const std::string &output_stage);

        void clear_reply();
    void setStage(seda::Stage::Ptr stage)
    {
      // assert stage->strategy() == this
      client_stage_ = stage;
    }

    std::string name_;
    seda::Stage::Ptr _output_stage;
    std::string _output_stage_name;

    boost::mutex mtx_;
    event_queue_t m_incoming_events;

    seda::Stage::Ptr client_stage_;
    ClientContext fsm_;

    // config variables
    timeout_t timeout_;
    std::string orchestrator_;
    std::string my_location_;
  };
}}

#endif
