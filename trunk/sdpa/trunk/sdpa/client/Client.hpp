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
#include <sdpa/client/ClientActions.hpp>

#include <sdpa/client/types.hpp>
#include <sdpa/client/exceptions.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <sdpa/client/generated/ClientFsm_sm.h>

namespace sdpa { namespace client {
  class Client : public ClientActions, public seda::Strategy {
  public:
    typedef sdpa::shared_ptr<Client> ptr_t;

    ~Client();

    // seda strategy
    void perform(const seda::IEvent::Ptr &);

    // API
    static Client::ptr_t create(const std::string &name_prefix="sdpac"
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

    const std::string &version() const
    {
      return version_;
    }

    const std::string &copyright() const
    {
      return copyright_;
    }

    const std::string &contact() const
    {
      return contact_;
    }

    const std::string &build_timestamp() const
    {
      return timestamp_;
    }

    const std::string &revision() const
    {
      return revision_;
    }
    const std::string &build() const
    {
      return build_;
    }

    void start(const config_t &cfg) throw(ClientException);
    void shutdown() throw(ClientException);
    void subscribe() throw (ClientException);

    job_id_t submitJob(const job_desc_t &) throw (ClientException);
    void cancelJob(const job_id_t &) throw (ClientException);
    std::string queryJob(const job_id_t &) throw (ClientException);
    void deleteJob(const job_id_t &) throw (ClientException);
    result_t retrieveResults(const job_id_t &) throw (ClientException);

    // Action implementations
    void action_configure(const config_t &);
    void action_config_ok();
    void action_config_nok();
    void action_shutdown();

    void action_subscribe(const seda::IEvent::Ptr&);
    void action_submit(const seda::IEvent::Ptr&);
    void action_cancel(const seda::IEvent::Ptr&);
    void action_query(const seda::IEvent::Ptr&);
    void action_retrieve(const seda::IEvent::Ptr&);
    void action_delete(const seda::IEvent::Ptr&);

    void action_store_reply(const seda::IEvent::Ptr &);

    void action_configure_network(const config_t &);
    void action_shutdown_network();

    const std::string &input_stage() const { return client_stage_->name(); }
    const std::string &output_stage() const { return output_stage_; }

    typedef unsigned long long timeout_t;
    seda::IEvent::Ptr wait_for_reply() throw (Timedout);
    seda::IEvent::Ptr wait_for_reply(timeout_t t) throw (Timedout);

  private:
	Client(const std::string &a_name, const std::string &output_stage);

    void setStage(seda::Stage::Ptr stage)
    {
      // assert stage->strategy() == this
      client_stage_ = stage;
    }

    std::string version_;
    std::string copyright_;
    std::string contact_;
    std::string timestamp_;
    std::string revision_;
    std::string build_;

    std::string name_;
    std::string output_stage_;

    boost::mutex mtx_;
    boost::condition_variable cond_;
    seda::IEvent::Ptr reply_;

    seda::Stage::Ptr client_stage_;
    ClientContext fsm_;

    // config variables
    timeout_t timeout_;
    std::string orchestrator_;
    std::string my_location_;
  };
}}

#endif
