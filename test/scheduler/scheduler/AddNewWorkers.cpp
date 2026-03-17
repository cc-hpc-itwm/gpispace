// Copyright (C) 2016,2018-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <test/scheduler/scheduler/utils.hpp>
#include <gspc/scheduler/types.hpp>

#include <gspc/testing/certificates_data.hpp>

#include <gspc/we/type/Activity.hpp>
#include <gspc/we/type/ModuleCall.hpp>
#include <gspc/we/type/Transition.hpp>
#include <gspc/we/type/net.hpp>
#include <gspc/we/type/place.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/optional.hpp>
#include <gspc/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <optional>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

static
gspc::we::type::Activity net_with_n_children (unsigned int n)
{
  gspc::we::type::property::type props;
  props.set ({"fhg", "drts", "schedule", "num_worker"}, std::to_string (1) + "UL");

  gspc::testing::unique_random<std::string> transition_names;

  std::vector<gspc::we::type::Transition> transitions;
  for (unsigned int k{0}; k < n; k++)
  {
    transitions.emplace_back
      ( transition_names()
      , gspc::we::type::ModuleCall
          ( gspc::testing::random_string()
          , gspc::testing::random_string()
          , std::unordered_map<std::string, gspc::we::type::MemoryBufferInfo>()
          , std::list<gspc::we::type::memory_transfer>()
          , std::list<gspc::we::type::memory_transfer>()
          , true
          , true
          )
      , std::nullopt
      , props
      , gspc::we::priority_type()
      , std::optional<gspc::we::type::eureka_id_type>{}
      , std::list<gspc::we::type::Preference>{}
      , gspc::we::type::track_shared{}
      );
  }

  const std::string port_name (gspc::testing::random_string());
  std::vector<gspc::we::port_id_type> port_ids_in;
  for (unsigned int k{0}; k < n; k++)
  {
    port_ids_in.emplace_back ( transitions.at (k).add_port
                                 ( gspc::we::type::Port ( port_name
                                                    , gspc::we::type::port::direction::In{}
                                                    , std::string ("string")
                                                    , gspc::we::type::property::type()
                                                    )
                                 )
                              );
  }

  gspc::we::type::net_type net;

  std::vector<gspc::we::place_id_type> place_ids_in;
  for (unsigned int k{0}; k < n; k++)
  {
    place_ids_in.emplace_back
      (net.add_place (gspc::we::type::place::type ( port_name + std::to_string (k)
                                  , std::string ("string")
                                  , std::nullopt
                                  , std::nullopt
                                  , gspc::we::type::property::type{}
                                  , gspc::we::type::place::type::Generator::No{}
                                  )
                     )
      );
  }

  for (unsigned int k{0}; k < n; k++)
  {
    net.put_value (place_ids_in.at (k), gspc::testing::random_string_without ("\\\""));
  }

  std::vector<gspc::we::transition_id_type> transition_ids;
  for (unsigned int k{0}; k < n; k++)
  {
    transition_ids.emplace_back (net.add_transition (transitions.at (k)));
    net.add_connection ( gspc::we::edge::PT{}
                        , transition_ids.at (k)
                        , place_ids_in.at (k)
                        , port_ids_in.at (k)
                        , gspc::we::type::property::type()
                        );
  }

  return gspc::we::type::Activity
    ( gspc::we::type::Transition ( gspc::testing::random_string()
                             , net
                             , std::nullopt
                             , gspc::we::type::property::type()
                             , gspc::we::priority_type()
                             , std::optional<gspc::we::type::eureka_id_type>{}
                             , std::list<gspc::we::type::Preference>{}
                             , gspc::we::type::track_shared{}
                             )
    );
}

// This case tests if each worker added after a user workflow submission
// gets assigned a task, provided sufficient activities are produced by the
// workflow engine
BOOST_DATA_TEST_CASE (add_new_workers, certificates_data, certificates)
{
  const utils::agent agent (certificates);
  utils::client client (agent, certificates);

  const unsigned int n_initial_workers (25);
  const unsigned int n_new_workers (25);

  std::vector<std::promise<std::string>> submit_events (n_initial_workers);

  std::vector<std::string> jobs;

  std::vector<std::unique_ptr<utils::fake_drts_worker_waiting_for_finished_ack>>
    workers;

  for (unsigned int i{0}; i < n_initial_workers; i++)
  {
    auto& e (submit_events.at (i));
    workers.emplace_back
      ( std::make_unique<utils::fake_drts_worker_waiting_for_finished_ack>
        ( [&e] (std::string str) {e.set_value (str);}
        , agent
        , certificates
        )
      );
  }

  gspc::scheduler::job_id_t job_id
    (client.submit_job (net_with_n_children (n_initial_workers + n_new_workers)));

  // all the initial workers are waiting to receive a job
  for (unsigned int i{0}; i < n_initial_workers; i++)
  {
    jobs.emplace_back (submit_events.at (i).get_future().get());
  }

  // new workers are added and wait for being served a job
  std::vector<std::unique_ptr<utils::fake_drts_worker_waiting_for_finished_ack>>
    new_workers;
  std::vector<std::promise<std::string>> new_submit_events (n_new_workers);

  for (unsigned int i{0}; i < n_new_workers; i++)
  {
    auto& e (new_submit_events.at (i));
    new_workers.emplace_back
      ( std::make_unique<utils::fake_drts_worker_waiting_for_finished_ack>
        ( [&e] (std::string str) {e.set_value (str);}
        , agent
        , certificates
        )
      );
  }

  // all new workers are waiting to receive a job
  std::vector<std::string> new_jobs;
  for (unsigned int i{0}; i < n_new_workers; i++)
  {
    new_jobs.emplace_back (new_submit_events.at (i).get_future().get());
  }

  // finish the jobs sent to the initial set of workers
  for (unsigned int i{0}; i < n_initial_workers; i++)
  {
    workers[i]->finish_and_wait_for_ack (jobs[i]);
  }

  // finish the jobs sent to the new set of workers
  for (unsigned int i{0}; i < n_new_workers; i++)
  {
    new_workers[i]->finish_and_wait_for_ack (new_jobs[i]);
  }

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), gspc::scheduler::status::FINISHED);
}
