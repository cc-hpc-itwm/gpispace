// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <drts/private/scheduler_types_implementation.hpp>

#include <sdpa/daemon/GetSchedulerType.hpp>
#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <testing/certificates_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/monomorphic/generators/xrange.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>
#include <memory>
#include <string>
#include <vector>

namespace
{
  auto const boolean_test_data {::boost::unit_test::data::make ({false, true})};
}

BOOST_DATA_TEST_CASE
  ( restart_worker_with_dummy_workflow
  , certificates_data * boolean_test_data
  , certificates
  , might_require_multiple_workers
  )
{
  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  auto const activity
    ( utils::module_call
        ( fhg::util::testing::random_string()
        , might_require_multiple_workers
        )
    );

  sdpa::job_id_t const job_id (client.submit_job (activity));

  sdpa::worker_id_t worker_id;

  {
    std::promise<void> job_submitted;

    const utils::fake_drts_worker_waiting_for_finished_ack worker
      ( [&job_submitted] (std::string) { job_submitted.set_value(); }
      , agent
      , certificates
      );

    worker_id = worker.name();

    job_submitted.get_future().wait();
  }

  std::promise<std::string> job_submitted_to_restarted_worker;
  utils::fake_drts_worker_waiting_for_finished_ack restarted_worker
    ( utils::reused_component_name (worker_id)
    , [&job_submitted_to_restarted_worker] (std::string s)
      { job_submitted_to_restarted_worker.set_value (s); }
    , agent
    , certificates
    );

  restarted_worker.finish_and_wait_for_ack
    (job_submitted_to_restarted_worker.get_future().get());

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id), sdpa::status::FINISHED);

  client.delete_job (job_id);
}

BOOST_DATA_TEST_CASE
  ( retry_job_a_specified_number_of_times
  , certificates_data
  , certificates
  )
{
  unsigned long const maximum_number_of_retries (2);

  utils::agent const agent (certificates);

  utils::client client (agent, certificates);
  auto const activity
    (utils::module_call_with_max_num_retries (maximum_number_of_retries));

  sdpa::job_id_t const job_id (client.submit_job (activity));

  for (unsigned long i {0}; i <= maximum_number_of_retries; ++i)
  {
    std::promise<void> job_submitted;

    const utils::fake_drts_worker_notifying_module_call_submission worker
      ( [&job_submitted] (std::string) { job_submitted.set_value(); }
      , agent
      , certificates
      );

    job_submitted.get_future().wait();
  }

  sdpa::client::job_info_t job_info;

  BOOST_REQUIRE_EQUAL
    (client.wait_for_terminal_state (job_id, job_info), sdpa::status::FAILED);

  BOOST_REQUIRE_EQUAL (job_info.error_message, "Number of retries exceeded!");

  client.delete_job (job_id);
}
