// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <testing/certificates_data.hpp>

#include <util-generic/latch.hpp>
#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

#include <future>
#include <string>

BOOST_DATA_TEST_CASE
  (restart_worker_with_dummy_workflow, certificates_data, certificates)
{
  const utils::agent agent (certificates);

  utils::client client (agent, certificates);
  sdpa::job_id_t const job_id (client.submit_job (utils::module_call()));

  sdpa::worker_id_t worker_id;

  {
    fhg::util::latch job_submitted (1);

    const utils::fake_drts_worker_waiting_for_finished_ack worker
      ( [&job_submitted] (std::string) { job_submitted.count_down(); }
      , agent
      , certificates
      );

    worker_id = worker.name();

    job_submitted.wait();
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
}
