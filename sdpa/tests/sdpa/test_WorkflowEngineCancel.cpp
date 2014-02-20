#define BOOST_TEST_MODULE WorkflowEngineCancel

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>
#include <cstdlib>
#include <ctime>

#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

BOOST_GLOBAL_FIXTURE (KVSSetup)

class TestAgent : public sdpa::daemon::Agent
 {
  public:
    TestAgent (const std::string& name
               , unsigned int n_finished_activities_before_cancel
               , unsigned int n_wait_activities_before_cancel)
      : Agent (name, "127.0.0.1", kvs_host(), kvs_port(), sdpa::master_info_list_t(), boost::none)
      , _n_received_activities(0)
      , _n_finished_activities_before_cancel(n_finished_activities_before_cancel)
      , _n_wait_activities_before_cancel(n_wait_activities_before_cancel)
    {}

    void submit ( const we::layer::id_type& activity_id
                , const we::type::activity_t& activity
                )
    {
      sdpa::daemon::GenericDaemon::submit(activity_id, activity);

      _n_received_activities++;

      if(_n_received_activities <= _n_finished_activities_before_cancel)
      {
          workflowEngine()->finished(activity_id, activity);
      }

      if (_n_received_activities == _n_wait_activities_before_cancel )
      {
        _cond_start_cancel.notify_one();
      }
    }

    void handleCancelJobEvent(const sdpa::events::CancelJobEvent* pEvt )
    {
      sdpa::daemon::Job* pJob = jobManager().findJob(pEvt->job_id());
      BOOST_REQUIRE(pJob);
      workflowEngine()->canceled(pEvt->job_id());
    }

    void canceled(const we::layer::id_type&)
    {
      _cond_workflow_canceled.notify_one();
    }

    void start_workflow_cancelation(const sdpa::job_id_t& job_id)
    {
      boost::unique_lock<boost::mutex> lock(_mtx_start_cancel);
      _cond_start_cancel.wait(lock);
      workflowEngine()->cancel (job_id);
    }

    void wait_workflow_canceled()
    {
      boost::unique_lock<boost::mutex> lock (_mtx_workflow_canceled);
      _cond_workflow_canceled.wait (lock);
    }

  private:

    boost::mutex _mtx_start_cancel;
    boost::mutex _mtx_workflow_canceled;
    boost::condition_variable_any _cond_start_cancel;
    boost::condition_variable_any _cond_workflow_canceled;
    unsigned int _n_received_activities;
    unsigned int _n_finished_activities_before_cancel;
    unsigned int _n_wait_activities_before_cancel;
 };

BOOST_AUTO_TEST_CASE (agent_cancels_workflow_after_first_activity_received)
{
  // Note: this workflow produces 30 activities
  const std::string workflow
      (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const we::type::activity_t activity (workflow);
  TestAgent agent ("agent_0", 0, 1);

  sdpa::job_id_t job_id(fhg::util::random_string());
  agent.workflowEngine()->submit (job_id, activity);

  agent.start_workflow_cancelation(job_id);

  agent.wait_workflow_canceled();
}

BOOST_AUTO_TEST_CASE (agent_cancels_workflow_after_first_activity_finished)
{
  // Note: this workflow produces 30 activities
  const std::string workflow
      (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const we::type::activity_t activity (workflow);
  TestAgent agent ("agent_0", 1, 5);

  sdpa::job_id_t job_id(fhg::util::random_string());
  agent.workflowEngine()->submit (job_id, activity);

  agent.start_workflow_cancelation(job_id);

  agent.wait_workflow_canceled();
}

BOOST_AUTO_TEST_CASE (agent_cancels_workflow_after_all_activities_submitted_and_finished)
{
  // Note: this workflow produces 30 activities
  const std::string workflow
      (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  const we::type::activity_t activity (workflow);
  TestAgent agent ("agent_0", 30, 30);

  sdpa::job_id_t job_id(fhg::util::random_string());
  agent.workflowEngine()->submit (job_id, activity);

  agent.start_workflow_cancelation(job_id);

  agent.wait_workflow_canceled();
}

BOOST_AUTO_TEST_CASE (agent_cancels_workflow_after_random_number_of_activities_submitted_or_finished)
{
  // Note: this workflow produces 30 activities
  const std::string workflow
      (utils::require_and_read_file ("workflows/coallocation_test.pnet"));

  srand (time(NULL));

  // generate a number between 1 and 30
  unsigned int n_act_submitted = rand() % 30 + 1;
  unsigned int n_act_finished  = rand() % n_act_submitted + 1;

  const we::type::activity_t activity (workflow);
  TestAgent agent ("agent_0", n_act_finished, n_act_submitted);

  sdpa::job_id_t job_id(fhg::util::random_string());
  agent.workflowEngine()->submit (job_id, activity);

  agent.start_workflow_cancelation(job_id);

  agent.wait_workflow_canceled();
}
