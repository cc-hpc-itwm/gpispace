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

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <exception>
#include <functional>
#include <list>
#include <memory>
#include <random>
#include <string>
#include <utility>

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
        ( sdpa::worker_id_t const& name
        , utils::agent const& master
        , sdpa::capabilities_set_t capabilities
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_component
          (name, master, capabilities, false, certificates)
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

  class fake_drts_worker_verifying_implementation
    : public utils::no_thread::basic_drts_worker
  {
  public:
    fake_drts_worker_verifying_implementation
        ( sdpa::worker_id_t const& name
        , utils::agent const& master
        , sdpa::capabilities_set_t capabilities
        , fhg::com::Certificates const& certificates
        , std::string const& expected_preference
        )
      : utils::no_thread::basic_drts_worker
          (name, master, capabilities, certificates)
      , _expected_preference (expected_preference)
    {}

    virtual ~fake_drts_worker_verifying_implementation()
    {
      BOOST_REQUIRE_EQUAL (_received_implementation, _expected_preference);
    }

    virtual void handleSubmitJobEvent
      ( fhg::com::p2p::address_t const& source
      , const sdpa::events::SubmitJobEvent* event
      ) override
    {
      _network.perform<sdpa::events::SubmitJobAckEvent>
        (source, *event->job_id());

      _received_implementation = event->implementation();

      finish_job (source, *event->job_id());
    }

    virtual void handleJobFinishedAckEvent
      ( fhg::com::p2p::address_t const&
      , const sdpa::events::JobFinishedAckEvent*
      ) override
    {}

  private:
    void finish_job
      ( fhg::com::p2p::address_t const& source
      , sdpa::job_id_t const& job
      )
    {
      _network.perform<sdpa::events::JobFinishedEvent>
        (source, job, we::type::activity_t());
    }

    boost::optional<std::string> _expected_preference;
    boost::optional<std::string> _received_implementation;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class fake_drts_worker_verifying_implementation_and_notifying_registration final
    : public fake_drts_worker_verifying_implementation
  {
  public:
    fake_drts_worker_verifying_implementation_and_notifying_registration
        ( sdpa::worker_id_t const& name
        , utils::agent const& master
        , sdpa::capabilities_set_t capabilities
        , fhg::com::Certificates const& certificates
        , std::string const& expected_preference
        , std::function<void ()> announce_registration_succeeded
        )
      : fake_drts_worker_verifying_implementation
          (name, master, capabilities, certificates, expected_preference)
      , _announce_registration_succeeded (announce_registration_succeeded)
      , _received_tasks (false)
    {}

    virtual ~fake_drts_worker_verifying_implementation_and_notifying_registration()
    {
      BOOST_CHECK (_received_tasks);
    }

    virtual void handle_worker_registration_response
      ( fhg::com::p2p::address_t const& source
      , sdpa::events::worker_registration_response const* response
      ) override
    {
      fake_drts_worker_verifying_implementation::handle_worker_registration_response
        (source, response);

      _announce_registration_succeeded();
    }

    virtual void handleSubmitJobEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent* e) override
    {
      fake_drts_worker_verifying_implementation::handleSubmitJobEvent
        (source, e);

      _received_tasks = true;
    }

  private:
    std::function<void ()> _announce_registration_succeeded;
    bool _received_tasks;
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
  fhg::util::testing::unique_random<sdpa::worker_id_t> generate_worker_id;

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

  auto const name (generate_worker_id());

  drts_component_observing_preferences observer
    ( name
    , agent
    , {sdpa::Capability (chosen_preference, name)}
    , certificates
    );

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
  unsigned int const max_num_workers (10);
  unsigned int const min_num_workers (2);
  unsigned int const num_preferences (3);

  fhg::util::testing::unique_random<std::string> generate_preference;
  fhg::util::testing::unique_random<sdpa::worker_id_t> generate_worker_id;

  Preferences preferences;

  utils::orchestrator const orchestrator (certificates);
  utils::agent const agent (orchestrator, certificates);

  std::list<std::unique_ptr<fake_drts_worker_verifying_implementation_and_notifying_registration>>
     workers;

  std::size_t n_total_workers (0);
  std::vector<std::size_t> num_workers (num_preferences);
  for (unsigned int k {0}; k < num_preferences; ++k)
  {
    num_workers[k] =
      ( (fhg::util::testing::random_integral<std::size_t>()
        % (max_num_workers - min_num_workers)
        )
      + min_num_workers
      );

    n_total_workers += num_workers[k];
  }

  fhg::util::latch to_register (n_total_workers);

  for (unsigned int i {0}; i < num_preferences; ++i)
  {
    auto const preference (generate_preference());
    preferences.emplace_back (preference);

    for (unsigned int k {0}; k < num_workers[i]; ++k)
    {
      auto const name (generate_worker_id());
      workers.emplace_back
        (fhg::util::cxx14::make_unique<fake_drts_worker_verifying_implementation_and_notifying_registration>
           ( name
           , agent
           , sdpa::capabilities_set_t {sdpa::Capability (preference, name)}
           , certificates
           , preference
           , [&to_register] { to_register.count_down(); }
           )
        );
    }
  }

  to_register.wait();

  utils::client client (orchestrator, certificates);

  sdpa::job_id_t const job
    (client.submit_job
       (net_with_n_children_and_preferences (n_total_workers, preferences))
    );

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state_and_cleanup (job), sdpa::status::FINISHED);
}
