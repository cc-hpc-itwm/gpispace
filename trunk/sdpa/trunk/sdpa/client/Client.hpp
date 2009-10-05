#ifndef SDPA_CLIENT_HPP
#define SDPA_CLIENT_HPP 1

#include <string>

#include <boost/thread.hpp> // condition variables

#include <seda/Stage.hpp>
#include <seda/Strategy.hpp>

#include <sdpa/sdpa-config.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/common.hpp>
#include <sdpa/types.hpp>
#include <sdpa/client/ClientActions.hpp>


#pragma GCC diagnostic ignored "-Wall"
#pragma GCC diagnostic ignored "-Wunused"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wunused-parameter"
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

    void action_submit(const seda::IEvent::Ptr&);
    void action_cancel(const seda::IEvent::Ptr&);
    void action_query(const seda::IEvent::Ptr&);
    void action_retrieve(const seda::IEvent::Ptr&);
    void action_delete(const seda::IEvent::Ptr&);

    void action_store_reply(const seda::IEvent::Ptr &);
  private:
    Client(const std::string &a_name, const std::string &output_stage)
      : seda::Strategy(a_name)
      , version_(SDPA_VERSION)
      , copyright_(SDPA_COPYRIGHT)
      , contact_(SDPA_CONTACT)
      , name_(a_name)
      , output_stage_(output_stage)
      , fsm_(*this)
    {
    
    }

    void setStage(seda::Stage::Ptr stage)
    {
      // assert stage->strategy() == this
      client_stage_ = stage;
    }

    typedef unsigned long long timeout_t;
    seda::IEvent::Ptr wait_for_reply(timeout_t timeout = 0);
    bool waitForReply();

    std::string version_;
    std::string copyright_;
    std::string contact_;

    std::string name_;
    std::string output_stage_;

    boost::mutex mtx_;
    boost::condition_variable cond_;
    seda::IEvent::Ptr reply_;
    bool blocked_;
    bool config_ok_;

    seda::Stage::Ptr client_stage_;
    ClientContext fsm_;
  };
}}

#endif
