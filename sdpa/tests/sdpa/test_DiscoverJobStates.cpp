#define BOOST_TEST_MODULE testDiscoverJobStates

#include "kvs_setup_fixture.hpp"
#include "tests_config.hpp"
#include <utils.hpp>

#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>

#include <we/layer.hpp>

#include <boost/ptr_container/ptr_list.hpp>
#include <boost/test/unit_test.hpp>
#include <sdpa/types.hpp>

#include <fhg/util/random_string.hpp>

#include <boost/thread/scoped_thread.hpp>

#include <deque>

namespace
{
  std::list<sdpa::status::code>
    get_leaf_job_info (const sdpa::discovery_info_t& discovery_result)
  {
    std::list<sdpa::status::code> list_info;

    if (!discovery_result.children().empty())
    {
      BOOST_FOREACH
        (const sdpa::discovery_info_t& child_info, discovery_result.children())
      {
        std::list<sdpa::status::code> const list_info_child
          (get_leaf_job_info (child_info));

        list_info.insert
          (list_info.end(), list_info_child.begin(), list_info_child.end());
      }
    }
    else
    {
      list_info.push_back (*discovery_result.state());
    }

    return list_info;
  }

  unsigned int max_depth (const sdpa::discovery_info_t& discovery_result)
  {
    unsigned int maxd = 1;

    BOOST_FOREACH
      (const sdpa::discovery_info_t& child_info, discovery_result.children())
    {
      unsigned int const depth (max_depth (child_info) + 1);

      if (maxd < depth)
      {
        maxd = depth;
      }
    }

    return maxd;
  }

  void check_has_one_leaf_job_with_expected_status
    ( const sdpa::discovery_info_t& discovery_result
    , const sdpa::status::code expected_status
    )
  {
     std::list<sdpa::status::code> const list_leaf_job_status
       (get_leaf_job_info (discovery_result));

     BOOST_REQUIRE_EQUAL (list_leaf_job_status.size(), 1);

     BOOST_FOREACH
       (const sdpa::status::code& leaf_job_status, list_leaf_job_status)
     {
       BOOST_REQUIRE_EQUAL (leaf_job_status, expected_status);
     }
  }
}

class Worker : public utils::BasicAgent
{
  public:
    Worker( const utils::agent& master_agent
          , const std::string& cpb_name
          , boost::optional<sdpa::status::code> reply_status)
      :  utils::BasicAgent (utils::random_peer_name(), master_agent, cpb_name)
      , _reply_status(reply_status)
    {}

    void handleSubmitJobEvent (const sdpa::events::SubmitJobEvent* pEvt)
    {
      sdpa::events::SubmitJobAckEvent::Ptr
      pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent( _name
                                                          , pEvt->from()
                                                          , *pEvt->job_id()));
      _network_strategy->perform (pSubmitJobAckEvt);
      _cond_got_job.notify_one();
    }

    void handleDiscoverJobStatesEvent (const sdpa::events::DiscoverJobStatesEvent *pEvt)
    {
      sdpa::discovery_info_t discovery_result( pEvt->job_id()
                                     , _reply_status
                                     , sdpa::discovery_info_set_t());


      sdpa::events::DiscoverJobStatesReplyEvent::Ptr
        pDiscRplEvt( new sdpa::events::DiscoverJobStatesReplyEvent ( _name
                                                                   , pEvt->from()
                                                                   , pEvt->discover_id()
                                                                   , discovery_result ));

      _network_strategy->perform (pDiscRplEvt);
    }

    void wait_for_jobs()
    {
      boost::unique_lock<boost::mutex> lock(_mtx_got_job);
      _cond_got_job.wait(lock);
    }

  private:
    boost::optional<sdpa::status::code> _reply_status;
    boost::mutex _mtx_got_job;
    boost::condition_variable_any _cond_got_job;
};


BOOST_GLOBAL_FIXTURE (KVSSetup)

namespace
{
  std::string get_next_discovery_id()
  {
    static int i (0);

    return (boost::format ("%1%%2%") % fhg::util::random_string() % i++).str();
  }

  void check_discover_worker_job_status (sdpa::status::code const reply_status)
  {
    const utils::orchestrator orchestrator (kvs_host(), kvs_port());
    const utils::agent agent (kvs_host(), kvs_port(), orchestrator);

    Worker worker (agent, "", reply_status);

    sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
    sdpa::job_id_t const job_id (client.submitJob (utils::module_call()));

    worker.wait_for_jobs();

    sdpa::discovery_info_t const discovery_result
      (client.discoverJobStates (get_next_discovery_id(), job_id));

    BOOST_REQUIRE_EQUAL (max_depth (discovery_result), 2);

    check_has_one_leaf_job_with_expected_status
      (discovery_result, reply_status);
  }
}

BOOST_AUTO_TEST_CASE (discover_worker_job_status)
{
  check_discover_worker_job_status (sdpa::status::FINISHED);
  check_discover_worker_job_status (sdpa::status::FAILED);
  check_discover_worker_job_status (sdpa::status::CANCELED);
  check_discover_worker_job_status (sdpa::status::PENDING);
  check_discover_worker_job_status (sdpa::status::RUNNING);
  check_discover_worker_job_status (sdpa::status::CANCELING);
}

BOOST_AUTO_TEST_CASE (discover_discover_inexistent_job)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());

  BOOST_REQUIRE_EQUAL
    ( sdpa::client::Client (orchestrator.name(), kvs_host(), kvs_port())
    . discoverJobStates ( fhg::util::random_string()
                        , fhg::util::random_string()
                        ).state()
    , boost::none
    );
}

BOOST_AUTO_TEST_CASE (discover_one_orchestrator_no_agent)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());

  check_has_one_leaf_job_with_expected_status
    ( client.discoverJobStates ( get_next_discovery_id()
                               , client.submitJob (utils::module_call())
                               )
    , sdpa::status::PENDING
    );
}

BOOST_AUTO_TEST_CASE (discover_one_orchestrator_one_agent)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (kvs_host(), kvs_port(), orchestrator);
  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (utils::module_call()));

  sdpa::discovery_info_t discovery_result;
  while (max_depth
          (discovery_result=client.discoverJobStates (get_next_discovery_id(), job_id)) !=2
        )
  {} // do nothing, discover again

  check_has_one_leaf_job_with_expected_status(discovery_result, sdpa::status::PENDING );
}

BOOST_AUTO_TEST_CASE (insufficient_number_of_workers)
{
  const utils::orchestrator orchestrator (kvs_host(), kvs_port());
  const utils::agent agent (kvs_host(), kvs_port(), orchestrator);

  sdpa::client::Client client (orchestrator.name(), kvs_host(), kvs_port());
  sdpa::job_id_t const job_id (client.submitJob (utils::module_call()));

  sdpa::discovery_info_t discovery_result;

  while (max_depth
           (discovery_result=client.discoverJobStates (get_next_discovery_id(), job_id)) !=2
         )
   {} // do nothing, discover again

  check_has_one_leaf_job_with_expected_status(discovery_result, sdpa::status::PENDING );
}

namespace
{
  class fake_drts_worker_discovering_running :
    public utils::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_discovering_running
        ( boost::function<void (std::string)> announce_job
        , std::string kvs_host
        , std::string kvs_port
        , utils::agent const& master
        )
      : utils::fake_drts_worker_notifying_module_call_submission
        (announce_job, kvs_host, kvs_port, master)
    {}

    virtual void handleDiscoverJobStatesEvent
      (const sdpa::events::DiscoverJobStatesEvent* e)
    {
      _network.perform
        ( sdpa::events::SDPAEvent::Ptr
          ( new sdpa::events::DiscoverJobStatesReplyEvent
            ( _name
            , e->from()
            , e->discover_id()
            , sdpa::discovery_info_t
              (e->job_id(), sdpa::status::RUNNING, sdpa::discovery_info_set_t())
            )
          )
        );
    }
  };

  std::size_t recursive_child_count (sdpa::discovery_info_t info)
  {
    std::size_t count (info.children().size());
    BOOST_FOREACH (sdpa::discovery_info_t child, info.children())
    {
      count += recursive_child_count (child);
    }
    return count;
  }

  struct wait_until_submitted_and_finish_on_scope_exit
  {
    wait_until_submitted_and_finish_on_scope_exit
        ( utils::fake_drts_worker_notifying_module_call_submission& worker
        , std::string expected_job_name
        , fhg::util::thread::event<std::string>& job_submitted
        )
      : _worker (worker)
      , _actual_job_name()
    {
      job_submitted.wait (_actual_job_name);
      BOOST_REQUIRE_EQUAL (_actual_job_name, expected_job_name);
    }
    ~wait_until_submitted_and_finish_on_scope_exit()
    {
      _worker.finish (_actual_job_name);
    }

    utils::fake_drts_worker_notifying_module_call_submission& _worker;
    std::string _actual_job_name;
  };

  void verify_child_count_in_agent_chain (const std::size_t num_agents)
  {
    const utils::orchestrator orchestrator (kvs_host(), kvs_port());
    boost::ptr_list<utils::agent> agents;
    agents.push_back (new utils::agent (kvs_host(), kvs_port(), orchestrator));

    for (std::size_t counter (1); counter < num_agents; ++counter)
    {
      agents.push_back
        (new utils::agent (kvs_host(), kvs_port(), agents.back()));
    }

    fhg::util::thread::event<std::string> job_submitted;

    fake_drts_worker_discovering_running worker
      ( boost::bind (&fhg::util::thread::event<std::string>::notify, &job_submitted, _1)
      , kvs_host(), kvs_port()
      , agents.back()
      );

    const std::string activity_name (fhg::util::random_string());

    utils::client::submitted_job submitted_job
      (utils::module_call (activity_name), orchestrator);

    const wait_until_submitted_and_finish_on_scope_exit _
      (worker, activity_name, job_submitted);

    BOOST_REQUIRE_EQUAL
      (recursive_child_count (submitted_job.discover()), num_agents);
  }
}

BOOST_AUTO_TEST_CASE (agent_chain_1)
{
  verify_child_count_in_agent_chain (1);
}
BOOST_AUTO_TEST_CASE (agent_chain_2)
{
  verify_child_count_in_agent_chain (2);
}
BOOST_AUTO_TEST_CASE (agent_chain_3_to_9)
{
  for (std::size_t n (3); n < 10; ++n)
  {
    verify_child_count_in_agent_chain (n);
  }
}
//! \note number of open files is the limiting factor
BOOST_AUTO_TEST_CASE (agent_chain_89)
{
  verify_child_count_in_agent_chain (89);
}
