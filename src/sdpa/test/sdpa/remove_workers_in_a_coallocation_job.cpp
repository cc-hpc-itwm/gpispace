// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <sdpa/test/sdpa/utils.hpp>
#include <sdpa/types.hpp>

#include <testing/certificates_data.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/monomorphic.hpp>
#include <boost/test/data/test_case.hpp>

#include <future>
#include <string>

BOOST_DATA_TEST_CASE
  ( remove_workers_in_a_coallocation_job_and_add_them_again
  , certificates_data
  , certificates
  )
{
  utils::agent const agent (certificates);

  utils::client client (agent, certificates);

  sdpa::job_id_t const job_id
    (client.submit_job (utils::net_with_one_child_requiring_workers (2)));

  {
    std::promise<std::string> submitted_0;
    utils::fake_drts_worker_waiting_for_finished_ack worker_0
      ([&] (std::string s) { submitted_0.set_value (s); }, agent, certificates);

    std::promise<std::string> submitted_1;
    utils::fake_drts_worker_notifying_module_call_submission worker_1
      ([&] (std::string s) { submitted_1.set_value (s); }, agent, certificates);

    worker_0.finish_and_wait_for_ack (submitted_0.get_future().get());
    submitted_1.get_future().get();
  }

  {
    std::promise<std::string> submitted_0;
    utils::fake_drts_worker_waiting_for_finished_ack worker_0
         ([&] (std::string s) { submitted_0.set_value (s); }, agent, certificates);

    std::promise<std::string> submitted_1;
    utils::fake_drts_worker_waiting_for_finished_ack worker_1
      ([&] (std::string s) { submitted_1.set_value (s); }, agent, certificates);

    worker_0.finish_and_wait_for_ack (submitted_0.get_future().get());
    worker_1.finish_and_wait_for_ack (submitted_1.get_future().get());
  }

  BOOST_REQUIRE_EQUAL ( client.wait_for_terminal_state_and_cleanup (job_id)
                      , sdpa::status::FINISHED
                      );
}
