// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>

#include <sdpa/job_states.hpp>

#define CHECK_SHOW(val, str)                                            \
  BOOST_AUTO_TEST_CASE (val)                                            \
  {                                                                     \
    BOOST_REQUIRE_EQUAL (sdpa::status::show (sdpa::status::val), str);  \
  }                                                                     \

CHECK_SHOW (PENDING,  "SDPA::Pending")
CHECK_SHOW (RUNNING, "SDPA::Running")
CHECK_SHOW (FINISHED, "SDPA::Finished")
CHECK_SHOW (FAILED, "SDPA::Failed")
CHECK_SHOW (CANCELED, "SDPA::Canceled")
CHECK_SHOW (CANCELING, "SDPA::Canceling")
