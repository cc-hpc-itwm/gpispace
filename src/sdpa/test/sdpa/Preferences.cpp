#include <sdpa/id_generator.hpp>
#include <sdpa/test/sdpa/utils.hpp>

#include <test/certificates_data.hpp>

#include <we/test/operator_equal.hpp>
#include <we/type/activity.hpp>

#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/latch.hpp>
#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/range/combine.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <cstddef>
#include <list>
#include <numeric>
#include <string>
#include <utility>
#include <vector>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (we::type::activity_t, os, activity)
{
  os << activity.to_string();
}

namespace
{
  class drts_component_observing_preferences final
    : public utils::basic_drts_component
  {
  public:
    // "Agent"
    drts_component_observing_preferences
        ( utils::orchestrator const& master
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_component (master, false, certificates)
    {}

    // "Worker"
    drts_component_observing_preferences
        ( utils::agent const& master
        , std::string capability
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_component (master, {capability}, false, certificates)
    {}

    template<typename Event>
      using Events
        = fhg::util::threadsafe_queue
            <std::pair<fhg::com::p2p::address_t, Event>>;

    Events<sdpa::events::SubmitJobEvent> jobs_submitted;

  private:
    virtual void handleSubmitJobEvent
      ( fhg::com::p2p::address_t const& source
      , sdpa::events::SubmitJobEvent const* event
      ) override
    {
      jobs_submitted.put (source, *event);
    }

    utils::basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class drts_component_observing_preferences_shared_queue final
    : public utils::basic_drts_component
  {
  public:
    using Queue
      = fhg::util::threadsafe_queue
          <std::pair<std::string, sdpa::events::SubmitJobEvent>>;

    drts_component_observing_preferences_shared_queue
        ( utils::agent const& master
        , fhg::com::Certificates const& certificates
        , std::string capability
        , Queue* jobs_submitted
        )
      : basic_drts_component (master, {capability}, false, certificates)
      , _jobs_submitted (jobs_submitted)
      , _capability (capability)
    {}

  private:
    virtual void handleSubmitJobEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::SubmitJobEvent const* event
      ) override
    {
      _jobs_submitted->put (_capability, *event);
    }

    Queue* _jobs_submitted;
    std::string _capability;
    utils::basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  we::type::transition_t transition_with_multiple_implementations
    (Preferences const& preferences)
  {
    fhg::util::testing::random<std::string> random_string;

    we::type::multi_module_call_t multi_mod;

    for (auto const& target : preferences)
    {
      using buffers
        = std::unordered_map<std::string, we::type::memory_buffer_info_t>;

      multi_mod.emplace
        ( target
        , we::type::module_call_t
            (random_string(), random_string(), buffers{}, {}, {}, true)
        );
    }

    return {random_string(), multi_mod, {}, {}, {}, {}, preferences};
  }

  we::type::activity_t activity_with_preferences
    ( std::list<std::string> const& preferences
    , boost::optional<unsigned long> worker_count = boost::none
    )
  {
    fhg::util::testing::random<std::string> random_string;

    auto transition (transition_with_multiple_implementations (preferences));

    if (worker_count)
    {
      transition.set_property ( {"fhg", "drts", "schedule", "num_worker"}
                              , std::to_string (*worker_count) + "UL"
                              );
    }

    const std::string port_name (random_string());
    transition.add_port
      ({port_name, we::type::PORT_IN, std::string ("string")});

    we::type::activity_t activity (transition);
    activity.add_input
      (port_name, fhg::util::testing::random_string_without ("\\\""));

    return activity;
  }

  we::type::activity_t net_with_n_children_and_preferences
    (unsigned int n, Preferences const& preferences)
  {
    fhg::util::testing::unique_random<std::string> place_names;
    fhg::util::testing::random<std::string> random_string;

    std::string const type ("string");

    we::type::net_type net;

    while (n --> 0)
    {
      auto transition (transition_with_multiple_implementations (preferences));

      auto const place_id (net.add_place ({place_names(), type, {}}));
      net.put_value
        ( place_id
        , random_string
            (fhg::util::testing::random<std::string>::except ("\\\""))
        );

      net.add_connection
        ( we::edge::PT
        , net.add_transition (transition)
        , place_id
        , transition.add_port ({random_string(), we::type::PORT_IN, type})
        , {}
        );
    }

    return we::type::activity_t
      (we::type::transition_t (random_string(), net, {}, {}, {}));
  }
}

BOOST_DATA_TEST_CASE
  ( agent_receives_the_expected_preferences_from_the_orchestrator
  , certificates_data
  , certificates
  )
{
  fhg::util::testing::unique_random<std::string> generate_preference;

  Preferences const preferences
    {generate_preference(), generate_preference(), generate_preference()};

  utils::orchestrator const orchestrator (certificates);
  drts_component_observing_preferences observer
    (orchestrator, certificates);

  auto const activity (activity_with_preferences (preferences));
  utils::client (orchestrator, certificates).submit_job (activity);

  auto const submitted (observer.jobs_submitted.get());
  BOOST_REQUIRE_EQUAL (submitted.second.activity(), activity);
}

BOOST_AUTO_TEST_CASE
  (preferences_are_properly_stored_in_requirements_and_preferences)
{
  auto const preferences
    ( fhg::util::testing::unique_randoms<std::list<we::type::preference_t>>
        (fhg::util::testing::random<std::size_t>{} (10, 1))
    );

  Requirements_and_preferences requirements_and_preferences
    ( {}
    , we::type::schedule_data()
    , null_transfer_cost
    , 0
    , 0
    , preferences
    );

  BOOST_REQUIRE_EQUAL
    (requirements_and_preferences.preferences(), preferences);
}

BOOST_DATA_TEST_CASE
  ( coallocation_for_jobs_with_multiple_implementations_not_allowed
  , certificates_data
  , certificates
  )
{
  fhg::util::testing::unique_random<std::string> preference_pool;

  Preferences const preferences
    {preference_pool(), preference_pool(), preference_pool()};

  utils::orchestrator const orchestrator (certificates);
  utils::agent const agent (orchestrator, certificates);

  utils::client client (orchestrator, certificates);

  auto const job ( client.submit_job
                     ( activity_with_preferences
                         ( preferences
                         , fhg::util::testing::random<unsigned long>{}
                             (std::numeric_limits<unsigned long>::max(), 2)
                         )
                     )
                 );

  sdpa::client::job_info_t info;

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job, info), sdpa::status::FAILED);

  BOOST_REQUIRE_EQUAL
    ( info.error_message
    , agent.name()
    + ": Not allowed to use coallocation for activities with "
      "multiple module implementations!"
    );

  client.delete_job (job);
}

BOOST_DATA_TEST_CASE
  ( worker_with_capabilty_matching_aribtrary_preference_receives_valid_implementation
  , certificates_data
  , certificates
  )
{
  fhg::util::testing::unique_random<std::string> generate_preference;

  utils::orchestrator const orchestrator (certificates);
  utils::agent const agent (orchestrator, certificates);

  Preferences const preferences
    {generate_preference(), generate_preference(), generate_preference()};

  auto const chosen_preference
    ( *std::next
        ( preferences.begin()
        , fhg::util::testing::random<std::size_t>{} (preferences.size() - 1)
        )
    );

  drts_component_observing_preferences observer
    (agent, chosen_preference, certificates);

  auto const activity (activity_with_preferences (preferences));
  utils::client (orchestrator, certificates).submit_job (activity);

  auto const submitted (observer.jobs_submitted.get());
  BOOST_REQUIRE_EQUAL (submitted.second.implementation(), chosen_preference);
}

BOOST_DATA_TEST_CASE
  ( variable_number_of_workers_and_tasks_with_preferences
  , certificates_data
  , certificates
  )
{
  unsigned int const max_num_workers_per_preference (10);
  unsigned int const min_num_workers_per_preference (2);
  unsigned int const num_preferences (3);

  fhg::util::testing::unique_random<std::string> generate_preference;

  utils::orchestrator const orchestrator (certificates);
  utils::agent const agent (orchestrator, certificates);

  drts_component_observing_preferences_shared_queue::Queue jobs_submitted;
  std::list<drts_component_observing_preferences_shared_queue> workers;

  Preferences preferences;
  std::vector<std::size_t> num_workers_per_preference;

  for (unsigned int i {0}; i < num_preferences; ++i)
  {
    preferences.emplace_back (generate_preference());

    num_workers_per_preference.emplace_back
      ( fhg::util::testing::random<std::size_t>{}
          (max_num_workers_per_preference, min_num_workers_per_preference)
      );

    for (unsigned int k {0}; k < num_workers_per_preference.back(); ++k)
    {
      workers.emplace_back
        ( agent
        , certificates
        , preferences.back()
        , &jobs_submitted
        );
    }
  }

  utils::client client (orchestrator, certificates);

  // For every preference, in order of most preferred to least, we
  // submit enough jobs to fill up all workers with the most preferred
  // capability left.
  for ( auto const& num_workers_and_preference
      : boost::combine (num_workers_per_preference, preferences)
      )
  {
    auto num_workers (boost::get<0> (num_workers_and_preference));
    auto const preference (boost::get<1> (num_workers_and_preference));

    client.submit_job
      (net_with_n_children_and_preferences (num_workers, preferences));

    while (num_workers --> 0)
    {
      auto const submission (jobs_submitted.get());
      // Worker actually supports that preference.
      BOOST_CHECK_EQUAL (submission.first, preference);
      // Worker was assigned the "best" preference.
      BOOST_CHECK_EQUAL
        (submission.second.implementation(), preference);
    }
  }
}
