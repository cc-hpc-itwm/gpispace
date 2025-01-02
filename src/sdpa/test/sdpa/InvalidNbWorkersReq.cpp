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

BOOST_DATA_TEST_CASE
  (invalid_number_of_workers_required, certificates_data, certificates)
{
  const utils::agent agent (certificates);
  utils::client client (agent, certificates);

  BOOST_REQUIRE_EQUAL
    ( client.wait_for_terminal_state_and_cleanup
        (client.submit_job (utils::net_with_one_child_requiring_workers (0)))
    , sdpa::status::FAILED
    );
}
