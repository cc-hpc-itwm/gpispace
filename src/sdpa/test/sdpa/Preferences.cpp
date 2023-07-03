// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/test/sdpa/utils.hpp>

#include <testing/certificates_data.hpp>

#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/testing/printer/list.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>

#include <we/type/Activity.hpp>

#include <fhg/util/next.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/range/combine.hpp>
#include <boost/test/data/test_case.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <future>
#include <limits>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

FHG_BOOST_TEST_LOG_VALUE_PRINTER (we::type::Activity, os, activity)
{
  os << activity.to_string();
}

namespace
{
  class drts_component_observing_preferences final
    : public utils::basic_drts_component
  {
  public:
    drts_component_observing_preferences
        ( utils::agent const& parent
        , std::string capability
        , fhg::com::Certificates const& certificates
        )
      : basic_drts_component (parent, {capability}, certificates)
    {}

    template<typename Event>
      using Events
        = fhg::util::threadsafe_queue<::boost::optional<std::string>>;

    Events<sdpa::events::SubmitJobEvent> implementations_submitted;

  private:
    void handleSubmitJobEvent
      ( fhg::com::p2p::address_t const& // source
      , sdpa::events::SubmitJobEvent const* event
      ) override
    {
      implementations_submitted.put (event->implementation());
    }

    utils::basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  we::type::Transition transition_with_multiple_implementations
    ( Preferences const& preferences
    , fhg::util::testing::unique_random<std::string>& unique_names
    )
  {
    fhg::util::testing::random<std::string> random_string;

    we::type::MultiModuleCall multi_mod;

    for (auto const& target : preferences)
    {
      using buffers
        = std::unordered_map<std::string, we::type::MemoryBufferInfo>;

      multi_mod.emplace
        ( target
        , we::type::ModuleCall
            (random_string(), random_string(), buffers{}, {}, {}, true, true)
        );
    }

    return {unique_names(), multi_mod, {}, {}, {}, {}, preferences};
  }

  we::type::Activity activity_with_preferences
    ( std::list<std::string> const& preferences
    , ::boost::optional<unsigned long> worker_count = ::boost::none
    )
  {
    fhg::util::testing::random<std::string> random_string;

    fhg::util::testing::unique_random<std::string> unique_names;
    auto transition
      (transition_with_multiple_implementations (preferences, unique_names));

    if (worker_count)
    {
      transition.set_property ( {"fhg", "drts", "schedule", "num_worker"}
                              , std::to_string (*worker_count) + "UL"
                              );
    }

    const std::string port_name (random_string());
    transition.add_port
      ({port_name, we::type::port::direction::In{}, std::string ("string"), we::type::property::type{}});

    we::type::Activity activity (transition);
    activity.add_input
      (port_name, fhg::util::testing::random_string_without ("\\\""));

    return activity;
  }

  we::type::Activity net_with_n_children_and_preferences
    (unsigned int n, Preferences const& preferences)
  {
    fhg::util::testing::unique_random<std::string> place_names;
    fhg::util::testing::random<std::string> random_string;
    fhg::util::testing::unique_random<std::string> unique_names;

    std::string const type ("string");

    we::type::net_type net;

    while (n --> 0)
    {
      auto transition
        (transition_with_multiple_implementations (preferences, unique_names));

      auto const place_id (net.add_place ({place_names(), type, {}, {}}));
      net.put_value
        ( place_id
        , random_string
            (fhg::util::testing::random<std::string>::except ("\\\""))
        );

      auto const port_id
        (transition.add_port ({random_string(), we::type::port::direction::In{}, type, we::type::property::type{}}));
      net.add_connection
        ( we::edge::PT{}
        , net.add_transition (std::move (transition))
        , place_id
        , port_id
        , {}
        );
    }

    return we::type::Activity
      (we::type::Transition (random_string(), net, {}, {}, {}
                              , ::boost::optional<we::type::eureka_id_type>{}
                              , std::list<we::type::Preference>{}
                              ));
  }
}

BOOST_AUTO_TEST_CASE
  (preferences_are_properly_stored_in_requirements_and_preferences)
{
  auto const preferences
    ( fhg::util::testing::unique_randoms<std::list<we::type::Preference>>
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

  utils::agent const agent (certificates);

  utils::client client (agent, certificates);

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

  BOOST_REQUIRE
    ( info.error_message.find
        ("Not allowed to use coallocation for activities with "
         "multiple module implementations!"
        )
    != std::string::npos
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

  utils::agent const agent (certificates);

  Preferences const preferences
    {generate_preference(), generate_preference(), generate_preference()};

  auto const chosen_preference
    ( *fhg::util::next
        ( preferences.begin()
        , fhg::util::testing::random<std::size_t>{} (preferences.size() - 1)
        )
    );

  drts_component_observing_preferences observer
    (agent, chosen_preference, certificates);

  auto const activity (activity_with_preferences (preferences));
  utils::client (agent, certificates).submit_job (activity);

  BOOST_REQUIRE_EQUAL
    (observer.implementations_submitted.get(), chosen_preference);
}

namespace
{
  class fake_drts_worker_notifying_implementation_reception
    : public utils::basic_drts_component
  {
  public:
    fake_drts_worker_notifying_implementation_reception
        ( utils::agent const& parent
        , fhg::com::Certificates const& certificates
        , std::string capability
        , std::promise<::boost::optional<std::string>>& job_submitted
        )
      : basic_drts_component (parent, {capability}, certificates)
      , _job_submitted (job_submitted)
    {}

    void handleSubmitJobEvent
      ( fhg::com::p2p::address_t const& source
      , const sdpa::events::SubmitJobEvent* event
      ) override
    {
      _network.perform<sdpa::events::SubmitJobAckEvent>
        (source, *event->job_id());

      _job_submitted.set_value (event->implementation());
    }

    void handleJobFinishedAckEvent
      ( fhg::com::p2p::address_t const&
      , const sdpa::events::JobFinishedAckEvent*
      ) override
    {}

  private:
    std::promise<::boost::optional<std::string>>& _job_submitted;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };
}

// worker_with_capabilty_matching_aribtrary_preference_receives_valid_implementation,
// but with more preferences and jobs, to verify that not just one
// random preference is correct but all of them.
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

  utils::agent const agent (certificates);

  std::list<std::promise<::boost::optional<std::string>>> jobs_submitted;
  std::list<fake_drts_worker_notifying_implementation_reception> workers;
  std::vector<std::string> expected_preferences;

  Preferences preferences;
  std::vector<std::size_t> num_workers_per_preference;
  std::size_t n_total_workers (0);

  for (unsigned int i {0}; i < num_preferences; ++i)
  {
    preferences.emplace_back (generate_preference());

    num_workers_per_preference.emplace_back
      ( fhg::util::testing::random<std::size_t>{}
          (max_num_workers_per_preference, min_num_workers_per_preference)
      );

    n_total_workers += num_workers_per_preference.back();

    for (unsigned int k {0}; k < num_workers_per_preference.back(); ++k)
    {
      expected_preferences.emplace_back (preferences.back());
      jobs_submitted.emplace_back();
      workers.emplace_back
        ( agent
        , certificates
        , preferences.back()
        , jobs_submitted.back()
        );
    }
  }

  utils::client client (agent, certificates);

  client.submit_job
    (net_with_n_children_and_preferences (n_total_workers, preferences));

  for ( auto const job_submitted_and_preference
      : ::boost::combine (jobs_submitted, expected_preferences)
      )
  {
    auto& job_submitted (::boost::get<0> (job_submitted_and_preference));
    auto const preference (::boost::get<1> (job_submitted_and_preference));

    auto const submission (job_submitted.get_future().get());
    BOOST_CHECK_EQUAL (submission, preference);
  }
}
