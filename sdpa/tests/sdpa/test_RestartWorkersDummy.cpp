#define BOOST_TEST_MODULE restart_worker_with_dummy_workflow

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

#include <fhg/util/random_string.hpp>

namespace {
  std::string gen_name() { return fhg::util::random_string_without (". "); }
}

class Worker : public utils::BasicAgent
{
  public:
    Worker (const std::string& name, const utils::agent& master_agent, const std::string cpb_name = "", bool notify_finished = false)
      :  utils::BasicAgent (name, master_agent, cpb_name)
      , _notify_finished(notify_finished)
    {}

    void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* pEvt)
    {
      sdpa::events::SubmitJobAckEvent::Ptr
      pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent( _name
                                                          , pEvt->from()
                                                          , *pEvt->job_id()));
      _network_strategy->perform (pSubmitJobAckEvt);

      if(_notify_finished)
      {
          sdpa::events::JobFinishedEvent::Ptr
          pJobFinishedEvt(new sdpa::events::JobFinishedEvent( _name
                                                            , pEvt->from()
                                                            , *pEvt->job_id()
                                                            , pEvt->description() ));

          _network_strategy->perform (pJobFinishedEvt);
      }
    }

    void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent* ){}

  private:
    bool _notify_finished;
};

BOOST_AUTO_TEST_CASE (restart_worker_with_dumm_workflow)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (orchestrator);

  sdpa::worker_id_t worker_id(gen_name());
  Worker* pWorker(new Worker(worker_id, agent));

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (utils::module_call()));

  delete pWorker;

  utils::fake_drts_worker_directly_finishing_jobs worker_0 (worker_id, agent);

  sdpa::client::job_info_t job_info;
  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state (job_id, job_info)
    , sdpa::status::FINISHED );
}
