#include <boost/test/unit_test.hpp>

#include <utils.hpp>

#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/types.hpp>

#include <we/layer.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random/string.hpp>

#include <boost/optional/optional_io.hpp>

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
  class fake_drts_worker_discovering final :
    public utils::no_thread::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_discovering
        ( std::function<void (std::string)> announce_job
        , utils::agent const& master
        , fhg::com::certificates_t const& certificates
        )
      : utils::no_thread::fake_drts_worker_notifying_module_call_submission
        (announce_job, master, certificates)
    {}

    virtual void handleDiscoverJobStatesEvent
      ( fhg::com::p2p::address_t const& source
      , const sdpa::events::DiscoverJobStatesEvent* e
      ) override
    {
      _network.perform<sdpa::events::DiscoverJobStatesReplyEvent>
        ( source
        , e->discover_id()
        , sdpa::discovery_info_t
            (e->job_id(), reply, sdpa::discovery_info_set_t())
        );
    }

  private:
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct wait_until_submitted_and_finish_on_scope_exit
  {
    wait_until_submitted_and_finish_on_scope_exit
        ( utils::no_thread::fake_drts_worker_notifying_module_call_submission& worker
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

    utils::no_thread::fake_drts_worker_notifying_module_call_submission& _worker;
    std::string _actual_job_name;
  };

  template<sdpa::status::code reply>
    void check_discover_worker_job_status
      ( fhg::log::Logger& logger
      , fhg::com::certificates_t const& certificates
      )
  {
    const utils::orchestrator orchestrator (logger, certificates);
    const utils::agent agent (orchestrator, logger, certificates);

    fhg::util::thread::event<std::string> job_submitted;

    fake_drts_worker_discovering<reply> worker
      ( [&job_submitted] (std::string j) { job_submitted.notify (j); }
      , agent
      , certificates
      );

    const std::string activity_name (fhg::util::testing::random_string());

    utils::client::submitted_job submitted_job
      (utils::module_call (activity_name), orchestrator, certificates);

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
  {
    check_discover_worker_job_status<sdpa::status::FINISHED> (_logger, boost::none);
    check_discover_worker_job_status<sdpa::status::FAILED> (_logger, boost::none);
    check_discover_worker_job_status<sdpa::status::CANCELED> (_logger, boost::none);
    check_discover_worker_job_status<sdpa::status::PENDING> (_logger, boost::none);
    check_discover_worker_job_status<sdpa::status::RUNNING> (_logger, boost::none);
    check_discover_worker_job_status<sdpa::status::CANCELING> (_logger, boost::none);
  }

  if (test_certificates)
  {
    check_discover_worker_job_status<sdpa::status::FINISHED> (_logger, test_certificates);
    check_discover_worker_job_status<sdpa::status::FAILED> (_logger, test_certificates);
    check_discover_worker_job_status<sdpa::status::CANCELED> (_logger, test_certificates);
    check_discover_worker_job_status<sdpa::status::PENDING> (_logger, test_certificates);
    check_discover_worker_job_status<sdpa::status::RUNNING> (_logger, test_certificates);
    check_discover_worker_job_status<sdpa::status::CANCELING> (_logger, test_certificates);
  }
}

void test_discover_discover_inexistent_job
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);

  BOOST_REQUIRE_EQUAL
    ( utils::client (orchestrator, certificates)
    . discover (fhg::util::testing::random_string()).state()
    , boost::none
    );
}

BOOST_FIXTURE_TEST_CASE (discover_discover_inexistent_job, setup_logging)
{
  test_discover_discover_inexistent_job (_logger, boost::none);
  test_discover_discover_inexistent_job (_logger, test_certificates);
}

void test_discover_one_orchestrator_no_agent
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);

  utils::client client (orchestrator, certificates);

  check_has_one_leaf_job_with_expected_status
    ( client.discover (client.submit_job (utils::module_call()))
    , sdpa::status::PENDING
    );
}

BOOST_FIXTURE_TEST_CASE (discover_one_orchestrator_no_agent, setup_logging)
{
  test_discover_one_orchestrator_no_agent (_logger, boost::none);

  if (test_certificates)
  {
    test_discover_one_orchestrator_no_agent (_logger, test_certificates);
  }
}

void test_discover_one_orchestrator_one_agent
  ( fhg::log::Logger& _logger
  , fhg::com::certificates_t const& certificates
  )
{
  const utils::orchestrator orchestrator (_logger, certificates);
  const utils::agent agent (orchestrator, _logger, certificates);

  utils::client client (orchestrator, certificates);
  sdpa::job_id_t const job_id (client.submit_job (utils::module_call()));

  sdpa::discovery_info_t discovery_result;
  while (max_depth (discovery_result = client.discover (job_id)) != 2)
  {} // do nothing, discover again

  check_has_one_leaf_job_with_expected_status(discovery_result, sdpa::status::PENDING );
}

BOOST_FIXTURE_TEST_CASE (discover_one_orchestrator_one_agent, setup_logging)
{
  test_discover_one_orchestrator_one_agent (_logger, boost::none);

  if (test_certificates)
  {
    test_discover_one_orchestrator_one_agent (_logger, test_certificates);
  }
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
    ( const std::size_t num_agents
    , fhg::log::Logger& logger
    , fhg::com::certificates_t const& certificates
    )
  {
    const utils::orchestrator orchestrator (logger, certificates);
    std::list<std::unique_ptr<utils::agent>> agents;
    agents.emplace_front
      (fhg::util::cxx14::make_unique<utils::agent> (orchestrator, logger, certificates));

    for (std::size_t counter (1); counter < num_agents; ++counter)
    {
      agents.emplace_front
        (fhg::util::cxx14::make_unique<utils::agent> (*agents.front(), logger, certificates));
    }

    fhg::util::thread::event<std::string> job_submitted;

    fake_drts_worker_discovering<sdpa::status::RUNNING> worker
      ( [&job_submitted] (std::string j) { job_submitted.notify (j); }
      , *agents.front()
      , certificates
      );

    const std::string activity_name (fhg::util::testing::random_string());

    utils::client::submitted_job submitted_job
      (utils::module_call (activity_name), orchestrator, certificates);

    const wait_until_submitted_and_finish_on_scope_exit _
      (worker, activity_name, job_submitted);

    sdpa::discovery_info_t const info (submitted_job.discover());

    BOOST_REQUIRE_EQUAL (recursive_child_count (info), num_agents);
    BOOST_REQUIRE_EQUAL (leaf_state (info, num_agents), sdpa::status::RUNNING);
  }
}

BOOST_FIXTURE_TEST_CASE (agent_chain_1, setup_logging)
{
  verify_child_count_in_agent_chain (1, _logger, boost::none);

  if (test_certificates)
  {
    verify_child_count_in_agent_chain (1, _logger, test_certificates);
  }
}
BOOST_FIXTURE_TEST_CASE (agent_chain_2, setup_logging)
{
  verify_child_count_in_agent_chain (2, _logger, boost::none);

  if (test_certificates)
  {
    verify_child_count_in_agent_chain (2, _logger, test_certificates);
  }
}
BOOST_FIXTURE_TEST_CASE (agent_chain_3_to_9, setup_logging)
{
  for (std::size_t n (3); n < 10; ++n)
  {
    verify_child_count_in_agent_chain (n, _logger, boost::none);
  }

  if (test_certificates)
  {
    for (std::size_t n (3); n < 10; ++n)
    {
      verify_child_count_in_agent_chain (n, _logger, test_certificates);
    }
  }
}
//! \note number of open files is the limiting factor
BOOST_FIXTURE_TEST_CASE (agent_chain_89, setup_logging)
{
  verify_child_count_in_agent_chain (89, _logger, boost::none);

  if (test_certificates)
  {
    verify_child_count_in_agent_chain (89, _logger, test_certificates);
  }
}
