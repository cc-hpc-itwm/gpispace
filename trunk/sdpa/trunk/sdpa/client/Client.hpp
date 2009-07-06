#ifndef SDPA_CLIENT_HPP
#define SDPA_CLIENT_HPP 1

#include <string>

#include <boost/thread.hpp>

#include <seda/Stage.hpp>
#include <seda/Strategy.hpp>

#include <sdpa/memory.hpp>
#include <sdpa/common.hpp>
#include <sdpa/types.hpp>
#include <sdpa/client/ClientActions.hpp>
#include <sdpa/client/ClientFsm_sm.h>
#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa { namespace client {
  class Client : public ClientActions, public seda::Strategy {
  public:
    typedef sdpa::shared_ptr<Client> ptr_t;
    typedef std::string config_t;
    typedef std::string result_t;

    ~Client();

    // seda strategy
    void perform(const seda::IEvent::Ptr &);

    // API
    static Client::ptr_t create(const std::string &name_prefix="sdpa.apps.client"
                              , const std::string &output_stage="sdpa.apps.client.out");

    void start(const config_t &cfg);
    void shutdown();

    job_id_t submitJob(const job_desc_t &);
    void cancelJob(const job_id_t &);
    int queryJob(const job_id_t &);
    void deleteJob(const job_id_t &);
    result_t retrieveResults(const job_id_t &);

    // Action implementations
    void action_configure(const config_t &);
    void action_config_ok();
    void action_config_nok();
    void action_shutdown();

    void action_submit(const job_desc_t &);
    void action_cancel(const job_id_t &);
    void action_query(const job_id_t &);
    void action_retrieve(const job_id_t &);
    void action_delete(const job_id_t &);
  private:
    Client(const std::string &name, const std::string &output_stage)
      : seda::Strategy(name)
      ,SDPA_INIT_LOGGER(name)
      ,name_(name)
      ,output_stage_(output_stage)
      ,fsm_(*this)
    {
    
    }

    void setStage(seda::Stage::Ptr stage)
    {
      // assert stage->strategy() == this
      client_stage_ = stage;
    }

    bool waitForReply();

    SDPA_DECLARE_LOGGER();

    std::string name_;
    std::string output_stage_;

    boost::mutex mtx_;
    boost::condition_variable cond_;
    sdpa::events::SDPAEvent::Ptr event_;
    bool blocked_;
    bool config_ok_;

    seda::Stage::Ptr client_stage_;
    ClientContext fsm_;
  };
}}

#endif
