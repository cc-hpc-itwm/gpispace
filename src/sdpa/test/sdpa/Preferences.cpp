#include <sdpa/id_generator.hpp>
#include <sdpa/test/sdpa/utils.hpp>

#include <test/certificates_data.hpp>

#include <we/layer.hpp>
#include <we/type/activity.hpp>

#include <util-generic/cxx14/make_unique.hpp>
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

namespace
{
  class drts_component_observing_preferences final
    : public utils::basic_drts_component
  {
  public:
    drts_component_observing_preferences
        ( utils::orchestrator const& master
        , fhg::com::Certificates const& certificates
        , std::list<std::string> const preferences
        )
      : basic_drts_component (utils::random_peer_name(), false, certificates)
      , workflow_engine
          ( std::bind ( &drts_component_observing_preferences::submit
                      , this
                      , std::placeholders::_1
                      , std::placeholders::_2
                      )
          , [](sdpa::job_id_t){}
          , [](sdpa::job_id_t, we::type::activity_t){}
          , [](sdpa::job_id_t, std::string){}
          , [](sdpa::job_id_t){}
          , [](sdpa::job_id_t, sdpa::job_id_t){}
          , [](sdpa::job_id_t, sdpa::discovery_info_t){}
          , [](std::string, boost::optional<std::exception_ptr>){}
          , []( std::string
              , boost::variant<std::exception_ptr
              , pnet::type::value::value_type>
              )
            {}
          , std::bind (&drts_component_observing_preferences::gen_id, this)
          , *_random_extraction_engine
          )
      , _expected_preferences (preferences)
    {
      _master = _network.connect_to (master.host(), master.port());

      _network.perform<sdpa::events::WorkerRegistrationEvent>
        ( _master.get()
        , _name
        , sdpa::capabilities_set_t()
        , fhg::util::testing::random<unsigned long>{}()
        , false
        , fhg::util::testing::random_string()
        );
    }

  private:
    virtual void handleSubmitJobEvent
      ( fhg::com::p2p::address_t const& source
      , const sdpa::events::SubmitJobEvent* e
      ) override
    {
      _network.perform<sdpa::events::SubmitJobAckEvent> (source, *e->job_id());
      _master = source;
      workflow_engine.submit (*e->job_id(), e->activity());
    }

    virtual void handleJobFinishedAckEvent
      ( fhg::com::p2p::address_t const&
      , const sdpa::events::JobFinishedAckEvent*
      ) override
    {}

    std::string gen_id() const
    {
      static sdpa::id_generator generator ("job");
      return generator.next();
    }

    void submit ( we::layer::id_type const& job_id
                , we::type::activity_t const& activity
                )
    {
      BOOST_REQUIRE_EQUAL (activity.preferences(), _expected_preferences);

      std::unique_ptr<sdpa::daemon::Job> const job
        (add_job (job_id, activity));

      BOOST_REQUIRE_EQUAL
        ( job->requirements_and_preferences().preferences()
        , _expected_preferences
        );

      finish_job (job_id, activity);
    }

    std::unique_ptr<sdpa::daemon::Job> add_job
      ( we::layer::id_type const& job_id
      , we::type::activity_t const& activity
      ) const
    {
      Requirements_and_preferences const requirements_and_preferences
        ( activity.requirements()
        , activity.get_schedule_data()
        , [] (std::string const&)
          {
            return fhg::util::testing::random_integral<unsigned int>();
          }
        , fhg::util::testing::random_integral<unsigned int>()
        , fhg::util::testing::random_integral<unsigned int>()
        , activity.preferences()
        );

      return fhg::util::cxx14::make_unique<sdpa::daemon::Job>
        ( std::move (job_id)
        , std::move (activity)
        , sdpa::daemon::job_source_wfe()
        , sdpa::daemon::job_handler_worker()
        , std::move (requirements_and_preferences)
        );
    }

    void finish_job
      ( sdpa::job_id_t const& job
      , we::type::activity_t const& activity
      )
    {
      _network.perform<sdpa::events::JobFinishedEvent>
        (*_master, job, activity);
    }

    boost::optional<std::mt19937> _random_extraction_engine;
    we::layer workflow_engine;
    std::list<std::string> _expected_preferences;
    utils::basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  we::type::activity_t activity_with_preferences
    (std::list<std::string> const& preferences, unsigned int n = 1)
  {
    we::type::property::type props;
    props.set ({"fhg", "drts", "schedule", "num_worker"}, std::to_string (n) + "UL");

    we::type::transition_t transition
      ( fhg::util::testing::random_string()
      , we::type::module_call_t ( fhg::util::testing::random_string()
                                , fhg::util::testing::random_string()
                                , std::unordered_map<std::string, std::string>()
                                , std::list<we::type::memory_transfer>()
                                , std::list<we::type::memory_transfer>()
                                )
      , boost::none
      , props
      , we::priority_type()
      , preferences
      );

    const std::string port_name (fhg::util::testing::random_string());
    transition.add_port ( we::type::port_t ( port_name
                                           , we::type::PORT_IN
                                           , std::string ("string")
                                           , we::type::property::type()
                                           )
                        );

    we::type::activity_t activity (transition, boost::none);
    activity.add_input ( transition.input_port_by_name (port_name)
                       , fhg::util::testing::random_string_without ("\\\"")
                       );

    return activity;
  }

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

    virtual void handleSubmitJobEvent
      ( fhg::com::p2p::address_t const& source
      , const sdpa::events::SubmitJobEvent* event
      ) override
    {
      _network.perform<sdpa::events::SubmitJobAckEvent>
        (source, *event->job_id());

      BOOST_REQUIRE (event->implementation());

      auto const activity_preferences (event->activity().preferences());

      BOOST_REQUIRE (!activity_preferences.empty());

      auto const corresponding_preference
        (std::find ( activity_preferences.begin()
                   , activity_preferences.end()
                   , *event->implementation()
                   )
        );

      BOOST_REQUIRE
        (corresponding_preference != activity_preferences.end());

      BOOST_REQUIRE_EQUAL (*corresponding_preference, _expected_preference);

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

    std::string _expected_preference;
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

  we::type::activity_t net_with_n_children_and_preferences
    (unsigned int n, Preferences const& preferences)
  {
    we::type::property::type props;
    props.set ({"fhg", "drts", "schedule", "num_worker"}, std::to_string (1) + "UL");

    std::vector<we::type::transition_t> transitions;
    for (unsigned int k{0}; k < n; k++)
    {
      transitions.emplace_back
        ( fhg::util::testing::random_string()
        , we::type::module_call_t ( fhg::util::testing::random_string()
                                  , fhg::util::testing::random_string()
                                  , std::unordered_map<std::string, std::string>()
                                  , std::list<we::type::memory_transfer>()
                                  , std::list<we::type::memory_transfer>()
                                  )
        , boost::none
        , props
        , we::priority_type()
        , preferences
        );
    }

    const std::string port_name (fhg::util::testing::random_string());
    std::vector<we::port_id_type> port_ids_in;
    for (unsigned int k{0}; k < n; k++)
    {
      port_ids_in.emplace_back ( transitions.at (k).add_port
                                   ( we::type::port_t ( port_name
                                                      , we::type::PORT_IN
                                                      , std::string ("string")
                                                      , we::type::property::type()
                                                      )
                                   )
                                );
    }

    we::type::net_type net;

    std::vector<we::place_id_type> place_ids_in;
    for (unsigned int k{0}; k < n; k++)
    {
      place_ids_in.emplace_back
        (net.add_place (place::type ( port_name + std::to_string (k)
                                    , std::string ("string")
                                    , boost::none
                                    )
                       )
        );
    }

    for (unsigned int k{0}; k < n; k++)
    {
      net.put_value (place_ids_in.at (k), fhg::util::testing::random_string_without ("\\\""));
    }

    std::vector<we::transition_id_type> transition_ids;
    for (unsigned int k{0}; k < n; k++)
    {
      transition_ids.emplace_back (net.add_transition (transitions.at (k)));
      net.add_connection ( we::edge::PT
                          , transition_ids.at (k)
                          , place_ids_in.at (k)
                          , port_ids_in.at (k)
                          , we::type::property::type()
                          );
    }

    return we::type::activity_t
      ( we::type::transition_t ( fhg::util::testing::random_string()
                               , net
                               , boost::none
                               , we::type::property::type()
                               , we::priority_type()
                               )
      , boost::none
      );
  }
}

BOOST_DATA_TEST_CASE
  ( agent_receives_the_expected_preferences_from_the_workflow_engine
  , certificates_data
  , certificates
  )
{
  fhg::util::testing::unique_random<std::string> generate_preference;

  Preferences const preferences
    {generate_preference(), generate_preference(), generate_preference()};

  utils::orchestrator const orchestrator (certificates);
  drts_component_observing_preferences const observer
    (orchestrator, certificates, preferences);

  utils::client client (orchestrator, certificates);

  sdpa::job_id_t const job
    (client.submit_job (activity_with_preferences (preferences)));

  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state_and_cleanup (job)
    , sdpa::status::FINISHED
    );
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

  sdpa::job_id_t const job
     (client.submit_job (activity_with_preferences (preferences, 2)));

  sdpa::client::job_info_t info;

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job, info), sdpa::status::FAILED);

  std::string const expected_error
    ("Not allowed to use coallocation for activities with multiple module implementations!");

  BOOST_REQUIRE
    (info.error_message.find (expected_error) != std::string::npos);

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

  Preferences const preferences
    {generate_preference(), generate_preference(), generate_preference()};

  utils::orchestrator const orchestrator (certificates);
  utils::agent const agent (orchestrator, certificates);

  auto const preference_index
    ( fhg::util::testing::random_integral<std::size_t>()
    % preferences.size()
    );

  auto const random_preference
    ( (preference_index == 0)
    ? preferences.begin()
    : std::next (preferences.begin(), preference_index)
    );

  auto const name (generate_worker_id());

  fake_drts_worker_verifying_implementation const worker
    ( name
    , agent
    , {sdpa::Capability (*random_preference, name)}
    , certificates
    , *random_preference
    );

  utils::client client (orchestrator, certificates);

  sdpa::job_id_t const job
    (client.submit_job (activity_with_preferences (preferences)));

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state_and_cleanup (job), sdpa::status::FINISHED);
}

BOOST_DATA_TEST_CASE
  ( variable_number_of_workers_and_tasks_with_preferences
  , certificates_data
  , certificates
  )
{
  unsigned int const max_num_workers (10);
  unsigned int const min_num_workers (2);
  unsigned int const min_num_tasks (100);
  unsigned int const num_preferences (3);

  fhg::util::testing::unique_random<std::string> generate_preference;
  fhg::util::testing::unique_random<sdpa::worker_id_t> generate_worker_id;

  Preferences preferences;

  utils::orchestrator const orchestrator (certificates);
  utils::agent const agent (orchestrator, certificates);

  std::list<std::unique_ptr<fake_drts_worker_verifying_implementation_and_notifying_registration>>
     workers;

  for (unsigned int i {0}; i < num_preferences; ++i)
  {
    auto const preference (generate_preference());
    preferences.emplace_back (preference);

    unsigned int const num_workers
      ( (fhg::util::testing::random_integral<std::size_t>()
        % (max_num_workers - min_num_workers)
        )
      + min_num_workers
      );

    for (unsigned int k {0}; k < num_workers; ++k)
    {
      auto const name (generate_worker_id());

      fhg::util::thread::event<> worker_registered;
      workers.emplace_back
        (fhg::util::cxx14::make_unique<fake_drts_worker_verifying_implementation_and_notifying_registration>
           ( name
           , agent
           , sdpa::capabilities_set_t {sdpa::Capability (preference, name)}
           , certificates
           , preference
           , [&worker_registered]() { worker_registered.notify(); }
           )
        );
      worker_registered.wait();
    }
  }

  utils::client client (orchestrator, certificates);

  unsigned int const num_tasks
    ( fhg::util::testing::random_integral<unsigned int>() % 100
    + min_num_tasks
    );

  sdpa::job_id_t const job
    (client.submit_job
       (net_with_n_children_and_preferences (num_tasks, preferences))
    );

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state_and_cleanup (job), sdpa::status::FINISHED);
}
