#define BOOST_TEST_MODULE TestCancelJob

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <boost/test/unit_test.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace test_agent_requests_workflow_cancelation_after_all_act_submitted
{
  class TestAgent : public sdpa::daemon::Agent
   {
    public:
      TestAgent (const std::string& name, unsigned int n_total_expected_activities)
        : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t(), boost::none)
        , _n_recv_tasks(0)
        , _n_total_expected_activities(n_total_expected_activities)
      {}

      void submit ( const we::layer::id_type& activity_id
                  , const we::type::activity_t& activity
                  )
      {
        sdpa::daemon::GenericDaemon::submit(activity_id, activity);

        boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
        if (++_n_recv_tasks == _n_total_expected_activities )
        {
          _cond_all_submitted.notify_one();
        }
      }

      void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt )
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
      unsigned int _n_total_expected_activities;
   };

  BOOST_AUTO_TEST_CASE (agent_requests_workflow_cancelation_after_all_act_submitted)
  {
    const std::string workflow
        (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

    const we::type::activity_t activity (workflow);
    // the workflow requires 2 activities
    TestAgent agent ("agent_0", 2);

    sdpa::job_id_t job_id(fhg::util::random_string());
    agent.workflowEngine()->submit (job_id, activity);
    agent.wait_all_submitted();

    agent.workflowEngine()->cancel (job_id);

    agent.wait_wf_canceled();
  }
}

namespace test_agent_requests_cancelation_after_activity_finshed
{
  class TestAgent : public sdpa::daemon::Agent
    {
    public:
      TestAgent (const std::string& name)
         : sdpa::daemon::Agent (name, "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t(), boost::none)
         , _n_recv_tasks(0)
     {}

     void submit ( const we::layer::id_type& activity_id
                       , const we::type::activity_t& activity
                       )
     {
       DLLOG (TRACE, _logger, "The layer submitted the job "<<activity_id);
       sdpa::daemon::GenericDaemon::submit(activity_id, activity);
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

     void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt )
     {
       sdpa::daemon::Job* pJob = jobManager().findJob(pEvt->job_id());
       BOOST_REQUIRE(pJob);
       DLLOG (TRACE, _logger, "Inform the layer that the activity "<<pEvt->job_id()<<" was successfully canceled!");
       workflowEngine()->canceled(pEvt->job_id());
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

  BOOST_AUTO_TEST_CASE (agent_requests_cancelation_after_activity_finshed)
  {
    const std::string workflow
      (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

    const we::type::activity_t activity (workflow);
    TestAgent agent ("agent_0");

    sdpa::job_id_t job_id(agent.gen_id());
    agent.workflowEngine()->submit (job_id, activity);

    agent.start_cancelation(job_id);

    agent.wait_wf_canceled();
  }
}

BOOST_AUTO_TEST_CASE (test_cancel_without_and_with_agent)
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

  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  BOOST_REQUIRE_EQUAL
    ( utils::client::submit_job_and_cancel_and_wait_for_termination
      (workflow, orchestrator)
    , sdpa::status::CANCELED
    );
}

BOOST_AUTO_TEST_CASE (test_call_cancel_twice_orch)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_0", "127.0.0.1", kvs_host(), kvs_port());

  const utils::agent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), orchestrator);

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  sdpa::job_id_t job_id(client.submitJob (workflow));
  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}

BOOST_AUTO_TEST_CASE (test_call_cancel_twice_orch_agent)
{
  const std::string workflow
    (utils::require_and_read_file ("workflows/coallocation_test2.pnet"));

  const utils::orchestrator orchestrator
    ("orchestrator_2", "127.0.0.1");

  utils::agent agent
      ("agent_2", "127.0.0.1", orchestrator);

  sdpa::client::Client client (orchestrator.name());
  sdpa::job_id_t job_id(client.submitJob (workflow));

  // wait until the agent gets the job, otherwise we're the previous case
  while(!agent._.hasJobs());

  client.cancelJob(job_id);

  BOOST_REQUIRE_EQUAL
  ( utils::client::wait_for_terminal_state (client, job_id)
    , sdpa::status::CANCELED );

  BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
}


namespace test_test_cancel_terminated_job
{
  class TestAgent : public sdpa::daemon::Agent
   {
   public:

     typedef boost::unordered_map< we::layer::id_type,
                                   we::type::activity_t > job_map_t ;

     TestAgent (const std::string& name, const std::string& master_name)
       : sdpa::daemon::Agent ( name,
                               "127.0.0.1",  kvs_host(), kvs_port(),
                               sdpa::master_info_list_t(1,
                               sdpa::MasterInfo(master_name)), 0, boost::none)
       , _notifier(boost::thread (&TestAgent::notify_failed, this))
     {
     }

     ~TestAgent()
     {
       _notifier.interrupt();
       if (_notifier.joinable())
       {
           _notifier.join();
       }
     }

     void submit ( const we::layer::id_type& activity_id
                 , const we::type::activity_t&
                 )
     {
       boost::unique_lock<boost::mutex> lock (_mtx);
       _job_list.push_back(activity_id);
       _cond.notify_one();
     }

     void notify_failed()
     {
       while(true)
       {
         boost::unique_lock<boost::mutex> lock (_mtx);
         _cond.wait(lock);
         BOOST_FOREACH(const sdpa::job_id_t& job_id, _job_list) {
           DLLOG(TRACE, _logger, "Notify the layer that the activity "<<job_id<<" failed!");
           workflowEngine()->failed(job_id, "");
         }
       }
     }

   private:
     boost::mutex _mtx;
     boost::condition_variable_any _cond;
     boost::thread _notifier;
     sdpa::job_id_list_t _job_list;
   };

  BOOST_AUTO_TEST_CASE (test_cancel_terminated_job)
  {
    const std::string workflow
      (utils::require_and_read_file ("workflows/capabilities.pnet"));
    const utils::orchestrator orchestrator ("orchestrator_3", "127.0.0.1",  kvs_host(), kvs_port());
    TestAgent agent("agent_3", "orchestrator_3");

    sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
    sdpa::job_id_t job_id(client.submitJob (workflow));

    BOOST_REQUIRE_EQUAL
      ( utils::client::wait_for_terminal_state (client, job_id)
          , sdpa::status::FAILED );

    BOOST_REQUIRE_THROW (client.cancelJob(job_id), std::runtime_error);
  }
}
