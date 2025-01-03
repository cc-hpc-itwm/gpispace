// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <testing/certificates_data.hpp>

#include <we/type/Activity.hpp>
#include <we/type/ModuleCall.hpp>
#include <we/type/Transition.hpp>
#include <we/type/net.hpp>
#include <we/type/place.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/optional.hpp>
#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>
#include <list>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

static
we::type::Activity net_with_n_children (unsigned int n)
{
  we::type::property::type props;
  props.set ({"fhg", "drts", "schedule", "num_worker"}, std::to_string (1) + "UL");

  fhg::util::testing::unique_random<std::string> transition_names;

  std::vector<we::type::Transition> transitions;
  for (unsigned int k{0}; k < n; k++)
  {
    transitions.emplace_back
      ( transition_names()
      , we::type::ModuleCall
          ( fhg::util::testing::random_string()
          , fhg::util::testing::random_string()
          , std::unordered_map<std::string, we::type::MemoryBufferInfo>()
          , std::list<we::type::memory_transfer>()
          , std::list<we::type::memory_transfer>()
          , true
          , true
          )
      , ::boost::none
      , props
      , we::priority_type()
      , ::boost::optional<we::type::eureka_id_type>{}
      , std::list<we::type::Preference>{}
      );
  }

  const std::string port_name (fhg::util::testing::random_string());
  std::vector<we::port_id_type> port_ids_in;
  for (unsigned int k{0}; k < n; k++)
  {
    port_ids_in.emplace_back ( transitions.at (k).add_port
                                 ( we::type::Port ( port_name
                                                    , we::type::port::direction::In{}
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
                                  , ::boost::none
                                  , we::type::property::type{}
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
    net.add_connection ( we::edge::PT{}
                        , transition_ids.at (k)
                        , place_ids_in.at (k)
                        , port_ids_in.at (k)
                        , we::type::property::type()
                        );
  }

  return we::type::Activity
    ( we::type::Transition ( fhg::util::testing::random_string()
                             , net
                             , ::boost::none
                             , we::type::property::type()
                             , we::priority_type()
                             , ::boost::optional<we::type::eureka_id_type>{}
                             , std::list<we::type::Preference>{}
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

  sdpa::job_id_t job_id
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
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);
}
