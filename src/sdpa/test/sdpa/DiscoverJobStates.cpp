#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <test/certificates_data.hpp>

#include <fhg/util/thread/event.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/optional.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <functional>
#include <list>
#include <memory>
#include <string>

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
    unsigned int maxd (1);

    for (auto const& child_info : discovery_result.children())
    {
      maxd = std::max (maxd, max_depth (child_info) + 1);
    }

    return maxd;
  }

#define REQUIRE_ONE_LEAF_WITH_STATUS(result_, expected_)                      \
  BOOST_REQUIRE_EQUAL                                                         \
    (get_leaf_job_info (result_), std::list<sdpa::status::code> {expected_})

  class fake_drts_worker_discovering final :
    public utils::no_thread::fake_drts_worker_notifying_module_call_submission
  {
  public:
    fake_drts_worker_discovering
        ( std::function<void (std::string)> announce_job
        , utils::agent const& master
        , fhg::com::Certificates const& certificates
        , sdpa::status::code reply
        )
      : utils::no_thread::fake_drts_worker_notifying_module_call_submission
        (announce_job, master, certificates)
      , _reply (reply)
      , _received_discover_request (false)
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
            (e->job_id(), _reply, sdpa::discovery_info_set_t())
        );

      _received_discover_request = true;
    }

    bool received_discover_request()
    {
      return _received_discover_request;
    }

  private:
    sdpa::status::code _reply;
    bool _received_discover_request;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  std::vector<sdpa::status::code> possible_status_codes()
  {
    using namespace sdpa::status;
    return {FINISHED, FAILED, CANCELED, PENDING, RUNNING, CANCELING};
  }
}

BOOST_DATA_TEST_CASE ( discover_worker_job_status
                     , certificates_data * possible_status_codes()
                     , certificates
                     , reply
                     )
{
  utils::agent const agent (certificates);

  fhg::util::thread::event<std::string> job_submitted;

  fake_drts_worker_discovering worker
    ( [&job_submitted] (std::string j) { job_submitted.notify (j); }
    , agent
    , certificates
    , reply
    );

  auto const activity_name (fhg::util::testing::random_string());

  utils::client client (agent, certificates);
  auto const job_id (client.submit_job (utils::module_call (activity_name)));
  BOOST_REQUIRE_EQUAL (job_submitted.wait(), activity_name);

  auto discovery_result (client.discover (job_id));

  // in case the agent didn't receive yet a submit ack for that job
  // from the worker and didn't update its state to RUNNING,
  // id doesn't forward the discovery request to the worker,
  // reporting the status PENDING
  while (!worker.received_discover_request())
  {
    REQUIRE_ONE_LEAF_WITH_STATUS (discovery_result, sdpa::status::PENDING);
    discovery_result = client.discover (job_id);
  }

  BOOST_REQUIRE_EQUAL (max_depth (discovery_result), 2);

  REQUIRE_ONE_LEAF_WITH_STATUS (discovery_result, reply);
}

BOOST_DATA_TEST_CASE (discover_inexistent_job, certificates_data, certificates)
{
  const utils::agent agent (certificates);

  BOOST_REQUIRE_EQUAL
    ( utils::client (agent, certificates)
    . discover (fhg::util::testing::random_string()).state()
    , boost::none
    );
}

BOOST_DATA_TEST_CASE
  (discover_one_agent, certificates_data, certificates)
{
  const utils::agent agent (certificates);

  utils::client client (agent, certificates);

  auto const job_id (client.submit_job (utils::module_call()));
  auto discovery_info (client.discover (job_id));

  while (discovery_info.children().empty())
  {
    BOOST_REQUIRE_EQUAL (discovery_info.state(), boost::none);
    discovery_info = client.discover (job_id);
  }

  REQUIRE_ONE_LEAF_WITH_STATUS (discovery_info, sdpa::status::PENDING);
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
}

//! \note number of open files is the limiting factor
//! * September 2019 - with new logging, more file descriptors are
//! used. Reduced chain from 89 to 76 to accommodate.
BOOST_DATA_TEST_CASE
  ( agent_chain
  , boost::unit_test::data::make ({1, 2, 3, 4, 5, 6, 7, 8, 9, 76})
  * certificates_data
  , num_agents
  , certificates
  )
{
  std::list<std::unique_ptr<utils::agent>> agents;
  agents.emplace_front
    (fhg::util::cxx14::make_unique<utils::agent> (certificates));

  for (int counter (1); counter < num_agents; ++counter)
  {
    agents.emplace_front
      (fhg::util::cxx14::make_unique<utils::agent> (*agents.front(), certificates));
  }

  fhg::util::thread::event<std::string> job_submitted;

  fake_drts_worker_discovering worker
    ( [&job_submitted] (std::string j) { job_submitted.notify (j); }
    , *agents.front()
    , certificates
    , sdpa::status::RUNNING
    );

  const std::string activity_name (fhg::util::testing::random_string());

  utils::client client (*agents.back(), certificates);
  auto const job_id (client.submit_job (utils::module_call (activity_name)));
  BOOST_REQUIRE_EQUAL (job_submitted.wait(), activity_name);

  auto info (client.discover (job_id));

  // in case the agent didn't receive yet a submit ack for that job
  // from the worker and didn't update its state to RUNNING,
  // id doesn't forward the discovery request to the worker,
  // reporting the status PENDING
  while (!worker.received_discover_request())
  {
    REQUIRE_ONE_LEAF_WITH_STATUS (info, sdpa::status::PENDING);
    info = client.discover (job_id);
  }

  BOOST_REQUIRE_EQUAL (recursive_child_count (info), num_agents);
  BOOST_REQUIRE_EQUAL (leaf_state (info, num_agents), sdpa::status::RUNNING);
}
