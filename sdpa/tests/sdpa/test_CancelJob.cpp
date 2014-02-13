#define BOOST_TEST_MODULE TestCancelJob

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace {
  const unsigned int n_total_expectd_activities  = 2;
}

namespace sdpa
{
  namespace daemon
  {
    class TestAgentAllActSubmitted : public Agent
    {
    public:
      TestAgentAllActSubmitted (const std::string& name)
        : Agent (name, "127.0.0.1", sdpa::master_info_list_t(), 0, boost::none)
          , _n_recv_tasks(0)
      {}

      void submit ( const we::layer::id_type& activity_id
                  , const we::type::activity_t& activity
                  )
      {
        GenericDaemon::submit(activity_id, activity);

        boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
        if (++_n_recv_tasks == n_total_expectd_activities )
        {
          _cond_all_submitted.notify_one();
        }
      }

      void handleCancelJobEvent(const events::CancelJobEvent* pEvt )
      {
        workflowEngine()->canceled(pEvt->job_id());
      }

      void canceled(const we::layer::id_type&)
      {
        _cond_wf_canceled.notify_one();
      }

      void wait_all_submitted()
      {
        boost::unique_lock<boost::mutex> lock (_mtx_all_submitted);
        _cond_all_submitted.wait (lock);
      }

      void wait_wf_canceled()
      {
        boost::unique_lock<boost::mutex> lock (_mtx_wf_canceled);
        _cond_wf_canceled.wait (lock);
      }

    private:

      boost::mutex _mtx_all_submitted;
      boost::mutex _mtx_wf_canceled;
      boost::condition_variable_any _cond_all_submitted;
      boost::condition_variable_any _cond_wf_canceled;
      unsigned int _n_recv_tasks;
    };
  }
}

BOOST_AUTO_TEST_CASE (agent_requests_workflow_cancelation_after_all_act_submitted)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

  const we::type::activity_t activity (workflow);
  sdpa::daemon::TestAgentAllActSubmitted agent ("agent_0");

  sdpa::job_id_t job_id(fhg::util::random_string());
  agent.workflowEngine()->submit (job_id, activity);
  agent.wait_all_submitted();

  agent.workflowEngine()->cancel (job_id);

  agent.wait_wf_canceled();
}

namespace sdpa
{
  namespace daemon
  {
    class TestAgentNoWait : public Agent
    {
    public:
      TestAgentNoWait (const std::string& name)
        : Agent (name, "127.0.0.1", sdpa::master_info_list_t(), 0, boost::none)
          , _n_recv_tasks(0)
      {}

      void submit ( const we::layer::id_type& activity_id
                        , const we::type::activity_t& activity
                        )
      {
        DLLOG (TRACE, _logger, "The layer submitted the job "<<activity_id);
        GenericDaemon::submit(activity_id, activity);
        if(_n_recv_tasks==0)
        {
           DLLOG (TRACE, _logger, "Inform the layer that the activity "<<activity_id<<" finished!");
           workflowEngine()->finished(activity_id, activity);
        }

        _n_recv_tasks++;
        boost::unique_lock<boost::mutex> const _ (_mtx_trigger_cancelation);
        if(_n_recv_tasks==5) // trigger cancelation after the 5th job submitted
        {
          _cond_trigger_cancelation.notify_one();
        }
      }

      void handleCancelJobEvent(const events::CancelJobEvent* pEvt )
      {
        Job* pJob = jobManager().findJob(pEvt->job_id());
        BOOST_REQUIRE(pJob!=NULL);
        if(!pJob)
        {
            if (pEvt->is_external())
            {
                DLLOG (TRACE, _logger, "Job "<<pEvt->job_id()<<" not found!");
            }
        }
        else
        {
          DLLOG (TRACE, _logger, "Inform the layer that the activity "<<pEvt->job_id()<<" was successfully canceled!");
          workflowEngine()->canceled(pEvt->job_id());
        }
      }

      void canceled(const we::layer::id_type& id)
      {
        DLLOG (TRACE, _logger, "The layer successfully canceled the job "<<id);
        _cond_wf_canceled.notify_one();
      }

      void wait_wf_canceled()
      {
        boost::unique_lock<boost::mutex> lock(_mtx_wf_canceled);
        _cond_wf_canceled.wait (lock);
      }

      void start_cancelation(const sdpa::job_id_t& job_id)
      {
        boost::unique_lock<boost::mutex> lock(_mtx_trigger_cancelation);
        _cond_trigger_cancelation.wait(lock);
        workflowEngine()->cancel (job_id);
      }

      std::string gen_id() { return GenericDaemon::gen_id(); }

    private:

      boost::mutex _mtx_all_submitted;
      boost::mutex _mtx_wf_canceled;
      boost::mutex _mtx_trigger_cancelation;
      boost::condition_variable_any _cond_all_submitted;
      boost::condition_variable_any _cond_wf_canceled;
      boost::condition_variable_any _cond_trigger_cancelation;
      unsigned int _n_recv_tasks;
    };
  }
}

BOOST_AUTO_TEST_CASE (agent_requests_cancelation_after_activity_finshed)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const we::type::activity_t activity (workflow);
  sdpa::daemon::TestAgentNoWait agent ("agent_0");

  sdpa::job_id_t job_id(agent.gen_id());
  agent.workflowEngine()->submit (job_id, activity);

  agent.start_cancelation(job_id);

  agent.wait_wf_canceled();
}

BOOST_AUTO_TEST_CASE (test_cancel_no_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_cancel_and_wait_for_termination
      (workflow, orchestrator)
    , sdpa::status::CANCELED
    );
}

BOOST_AUTO_TEST_CASE (test_cance_orch_and_agent_no_worker)
{
  const std::string workflow
    (utils::require_and_read_file ("transform_file.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_cancel_and_wait_for_termination
      (workflow, orchestrator)
    , sdpa::status::CANCELED
    );
}

BOOST_AUTO_TEST_CASE (test_call_cancel_twice)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_1
    ( "drts_1", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t job_id(client.submitJob (workflow));
  client.cancelJob(job_id);
  sdpa::client::job_info_t UNUSED_job_info;
  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state (job_id, UNUSED_job_info)
    , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (test_call_cancel_with_polling_client)
{
  const std::string workflow
    (utils::require_and_read_file ("capabilities.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_1
    ( "drts_1", agent
    , ""
    , TESTS_TRANSFORM_FILE_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t job_id(client.submitJob (workflow));

  client.cancelJob(job_id);
  sdpa::client::job_info_t UNUSED_job_info;
  client.wait_for_terminal_state_polling (job_id, UNUSED_job_info);
  BOOST_REQUIRE_EQUAL(client.queryJob(job_id), sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (test_cancel_terminated_job)
{
  const std::string workflow
    (utils::require_and_read_file ("capabilities.pnet"));

  const utils::orchestrator orchestrator ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());
  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  const utils::drts_worker worker_0
    ( "drts_0", agent
    , "A"
    , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_1
    ( "drts_1", agent
    , "B"
    , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
    , kvs_host(), kvs_port()
    );
  const utils::drts_worker worker_2
    ( "drts_2", agent
    , "A"
    , TESTS_EXAMPLE_CAPABILITIES_MODULES_PATH
    , kvs_host(), kvs_port()
    );

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t job_id(client.submitJob (workflow));

  sdpa::client::job_info_t UNUSED_job_info;
  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state (job_id, UNUSED_job_info)
        , sdpa::status::FINISHED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}
