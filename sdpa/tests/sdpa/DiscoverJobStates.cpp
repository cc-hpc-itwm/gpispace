#define BOOST_TEST_MODULE testDiscoverJobStates
#include <boost/test/unit_test.hpp>

#include <utils.hpp>

#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/types.hpp>

#include <we/layer.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/random_string.hpp>

#include <boost/ptr_container/ptr_list.hpp>

namespace
{
  std::list<sdpa::status::code>
    get_leaf_job_info (const sdpa::discovery_info_t& discovery_result)
  {
    std::list<sdpa::status::code> list_info;

    if (!discovery_result.children().empty())
    {
      for (const sdpa::discovery_info_t& child_info : discovery_result.children())
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

    for (const sdpa::discovery_info_t& child_info : discovery_result.children())
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

     for (const sdpa::status::code& leaf_job_status : list_leaf_job_status)
     {
       BOOST_REQUIRE_EQUAL (leaf_job_status, expected_status);
     }
  }

  template<sdpa::status::code reply>
  class fake_drts_worker_discovering :
    public utils::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_discovering
        ( std::function<void (std::string)> announce_job
        , utils::agent const& master
        )
      : utils::fake_drts_worker_notifying_module_call_submission
        (announce_job, master)
    {}

    virtual void handleDiscoverJobStatesEvent
      ( fhg::com::p2p::address_t const& source
      , const sdpa::events::DiscoverJobStatesEvent* e
      ) override
    {
      _network.perform
        ( source
        , sdpa::events::SDPAEvent::Ptr
          ( new sdpa::events::DiscoverJobStatesReplyEvent
            ( e->discover_id()
            , sdpa::discovery_info_t
              (e->job_id(), reply, sdpa::discovery_info_set_t())
            )
          )
        );
    }
  };

  struct wait_until_submitted_and_finish_on_scope_exit
  {
    wait_until_submitted_and_finish_on_scope_exit
        ( utils::fake_drts_worker_notifying_module_call_submission& worker
        , std::string expected_job_name
        , fhg::util::thread::event<std::string>& job_submitted
        )
      : _worker (worker)
      , _actual_job_name (job_submitted.wait())
    {
      BOOST_REQUIRE_EQUAL (_actual_job_name, expected_job_name);
    }
    ~wait_until_submitted_and_finish_on_scope_exit()
    {
      _worker.finish (_actual_job_name);
    }

    utils::fake_drts_worker_notifying_module_call_submission& _worker;
    std::string _actual_job_name;
  };

  template<sdpa::status::code reply>
    void check_discover_worker_job_status (fhg::log::Logger& logger)
  {
    const utils::orchestrator orchestrator (logger);
    const utils::agent agent (orchestrator, logger);

    fhg::util::thread::event<std::string> job_submitted;

    fake_drts_worker_discovering<reply> worker
      ( [&job_submitted] (std::string j) { job_submitted.notify (j); }
      , agent
      );

    const std::string activity_name (fhg::util::random_string());

    utils::client::submitted_job submitted_job
      (utils::module_call (activity_name), orchestrator);

    const wait_until_submitted_and_finish_on_scope_exit _
      (worker, activity_name, job_submitted);

    sdpa::discovery_info_t const discovery_result
      (submitted_job.discover());

    BOOST_REQUIRE_EQUAL (max_depth (discovery_result), 2);

    check_has_one_leaf_job_with_expected_status (discovery_result, reply);
  }
}

BOOST_FIXTURE_TEST_CASE (discover_worker_job_status, setup_logging)
{
  check_discover_worker_job_status<sdpa::status::FINISHED> (_logger);
  check_discover_worker_job_status<sdpa::status::FAILED> (_logger);
  check_discover_worker_job_status<sdpa::status::CANCELED> (_logger);
  check_discover_worker_job_status<sdpa::status::PENDING> (_logger);
  check_discover_worker_job_status<sdpa::status::RUNNING> (_logger);
  check_discover_worker_job_status<sdpa::status::CANCELING> (_logger);
}

BOOST_FIXTURE_TEST_CASE (discover_discover_inexistent_job, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);

  BOOST_REQUIRE_EQUAL
    ( utils::client (orchestrator)
    . discover (fhg::util::random_string()).state()
    , boost::none
    );
}

BOOST_FIXTURE_TEST_CASE (discover_one_orchestrator_no_agent, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);

  utils::client client (orchestrator);

  check_has_one_leaf_job_with_expected_status
    ( client.discover (client.submit_job (utils::module_call()))
    , sdpa::status::PENDING
    );
}

BOOST_FIXTURE_TEST_CASE (discover_one_orchestrator_one_agent, setup_logging)
{
  const utils::orchestrator orchestrator (_logger);
  const utils::agent agent (orchestrator, _logger);

  utils::client client (orchestrator);
  sdpa::job_id_t const job_id (client.submit_job (utils::module_call()));

  sdpa::discovery_info_t discovery_result;
  while (max_depth (discovery_result = client.discover (job_id)) != 2)
  {} // do nothing, discover again

  check_has_one_leaf_job_with_expected_status(discovery_result, sdpa::status::PENDING );
}

namespace
{
  std::size_t recursive_child_count (sdpa::discovery_info_t info)
  {
    std::size_t count (info.children().size());
    for (sdpa::discovery_info_t child : info.children())
    {
      count += recursive_child_count (child);
    }
    return count;
  }
  boost::optional<sdpa::status::code>
    leaf_state (sdpa::discovery_info_t info, std::size_t n)
  {
    sdpa::discovery_info_t i (info);

    while (n --> 0)
    {
      BOOST_REQUIRE_EQUAL (i.children().size(), 1);
      i = *i.children().begin();
    }

    return i.state();
  }

  void verify_child_count_in_agent_chain
    (const std::size_t num_agents, fhg::log::Logger& logger)
  {
    const utils::orchestrator orchestrator (logger);
    boost::ptr_list<utils::agent> agents;
    agents.push_back (new utils::agent (orchestrator, logger));

    for (std::size_t counter (1); counter < num_agents; ++counter)
    {
      agents.push_back (new utils::agent (agents.back(), logger));
    }

    fhg::util::thread::event<std::string> job_submitted;

    fake_drts_worker_discovering<sdpa::status::RUNNING> worker
      ( [&job_submitted] (std::string j) { job_submitted.notify (j); }
      , agents.back()
      );

    const std::string activity_name (fhg::util::random_string());

    utils::client::submitted_job submitted_job
      (utils::module_call (activity_name), orchestrator);

    const wait_until_submitted_and_finish_on_scope_exit _
      (worker, activity_name, job_submitted);

    sdpa::discovery_info_t const info (submitted_job.discover());

    BOOST_REQUIRE_EQUAL (recursive_child_count (info), num_agents);
    BOOST_REQUIRE_EQUAL (leaf_state (info, num_agents), sdpa::status::RUNNING);
  }
}

BOOST_FIXTURE_TEST_CASE (agent_chain_1, setup_logging)
{
  verify_child_count_in_agent_chain (1, _logger);
}
BOOST_FIXTURE_TEST_CASE (agent_chain_2, setup_logging)
{
  verify_child_count_in_agent_chain (2, _logger);
}
BOOST_FIXTURE_TEST_CASE (agent_chain_3_to_9, setup_logging)
{
  for (std::size_t n (3); n < 10; ++n)
  {
    verify_child_count_in_agent_chain (n, _logger);
  }
}
//! \note number of open files is the limiting factor
BOOST_FIXTURE_TEST_CASE (agent_chain_89, setup_logging)
{
  verify_child_count_in_agent_chain (89, _logger);
}
