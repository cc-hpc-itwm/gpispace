#define BOOST_TEST_MODULE TestWorkflowEngineDiscover

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <we/layer.hpp>

#include <boost/test/unit_test.hpp>
#include <sdpa/types.hpp>
#include <fhg/util/random_string.hpp>
#include <deque>

namespace
{
  bool has_state_pending (sdpa::discovery_info_t const& disc_res)
  {
    return disc_res.state() && disc_res.state().get() == sdpa::status::PENDING;
  }

  bool all_childs_are_pending (sdpa::discovery_info_t const& disc_res)
  {
    BOOST_FOREACH
      (const sdpa::discovery_info_t& child_info, disc_res.children())
    {
      if (!has_state_pending (child_info))
      {
        return false;
      }
    }

    return true;
  }

  bool has_two_childs_that_are_pending (sdpa::discovery_info_t const& disc_res)
  {
    return disc_res.children().size() == 2 && all_childs_are_pending (disc_res);
  }

  bool has_children_and_all_children_are_pending
    (sdpa::discovery_info_t const& disc_res)
  {
    return !disc_res.children().empty() && all_childs_are_pending (disc_res);
  }

  sdpa::events::ErrorEvent::Ptr create_disconnect_event (const sdpa::worker_id_t& worker_id,
                                                         const sdpa::worker_id_t& agent_id)
  {
    sdpa::events::ErrorEvent::Ptr pErrEvt(
          new  sdpa::events::ErrorEvent( worker_id
                                        , agent_id
                                        , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                        , std::string("worker ")+worker_id+" went down!" ));
    return pErrEvt;
  }

}

BOOST_GLOBAL_FIXTURE (KVSSetup)

class TestAgent : public sdpa::daemon::Agent
{
public:
  TestAgent ( const std::string& name
            , const std::string& url
            , std::string kvs_host, std::string kvs_port
            , const sdpa::master_info_list_t arrMasterNames
            )
    : Agent (name, url, kvs_host, kvs_port, arrMasterNames, boost::none)
      , _n_submitted_activities(0)
  {}

  std::string gen_id()
  {
    return GenericDaemon::gen_id();
  }

  void discover (we::layer::id_type discover_id, we::layer::id_type job_id)
  {
    boost::unique_lock<boost::mutex> const _ (_mtx_disc);
    _jobs_to_discover.push_back (std::make_pair (discover_id, job_id));
    _cond_disc.notify_all();
  }

  void discovered
    (we::layer::id_type disc_id, sdpa::discovery_info_t discover_result)
  {
    boost::unique_lock<boost::mutex> const _ (_mtx_result);
    _discovery_result = discover_result;
    _discover_id = disc_id;
    _cond_result.notify_one();
  }

  void submit ( const we::layer::id_type& activityId
              , const we::type::activity_t& activity
              )
  {
    GenericDaemon::submit (activityId, activity);

    boost::unique_lock<boost::mutex> const _ (_mtx_all_submitted);
    if (++_n_submitted_activities == 2)
    {
      _cond_all_submitted.notify_one();
    }
  }

  void wait_all_submitted()
  {
    boost::unique_lock<boost::mutex> lock(_mtx_all_submitted);
    _cond_all_submitted.wait (lock);
  }

  void notify_discovered()
  {
    while (true)
    {
      boost::unique_lock<boost::mutex> lock(_mtx_disc);
      _cond_disc.timed_wait (lock, boost::posix_time::seconds (1));

      if (!_jobs_to_discover.empty())
      {
        std::pair<we::layer::id_type, sdpa::job_id_t> const pair
          (_jobs_to_discover.front());
        _jobs_to_discover.pop_front();

        sdpa::daemon::Job* const pJob (jobManager().findJob (pair.second));

        sdpa::discovery_info_t const discover_result
          ( pair.second
          , pJob
          ? boost::optional<sdpa::status::code> (pJob->getStatus())
          : boost::none
          , sdpa::discovery_info_set_t()
          );

        workflowEngine()->discovered (pair.first, discover_result);
      }
    }
  }

  we::layer::id_type wait_for_discovery_result()
  {
    boost::unique_lock<boost::mutex> lock_res (_mtx_result);
    _cond_result.wait (lock_res);
    return _discover_id;
  }

  bool has_two_pending_children()
  {
    return _discovery_result.children().size() >= 2
      && all_childs_are_pending (_discovery_result);
  }

private:
  boost::mutex _mtx_disc;
  boost::mutex _mtx_result;
  boost::mutex _mtx_all_submitted;
  boost::condition_variable_any _cond_disc;
  boost::condition_variable_any _cond_result;
  boost::condition_variable_any _cond_all_submitted;
  std::deque<std::pair<we::layer::id_type, sdpa::job_id_t> > _jobs_to_discover;
  sdpa::discovery_info_t _discovery_result;
  we::layer::id_type _discover_id;
  unsigned int _n_submitted_activities;
};

BOOST_AUTO_TEST_CASE (test_discover_activities)
{
  const std::string workflow
    (utils::require_and_read_file ("coallocation_test2.pnet"));

  const we::type::activity_t activity (workflow);

  sdpa::master_info_list_t listMasterInfo;
  TestAgent agent
    ("agent_0", "127.0.0.1", kvs_host(), kvs_port(), listMasterInfo);

  we::layer::id_type const id ("test_job");

  agent.workflowEngine()->submit (id, activity);
  //! Note it seems to have a race condition somewhere if we call layer submit and
  // layer discover immediately (deadlock)
  agent.wait_all_submitted();

  boost::thread thrd_notify
    (boost::thread (&TestAgent::notify_discovered, &agent));

  while (!agent.has_two_pending_children())
  {
    we::layer::id_type const disc_id(fhg::util::random_string());
    agent.workflowEngine()->discover (disc_id, id);
    BOOST_REQUIRE_EQUAL (agent.wait_for_discovery_result(), disc_id);
  }

  thrd_notify.interrupt();
  if (thrd_notify.joinable())
  {
    thrd_notify.join();
  }
}
