// Copyright (C) 2012-2016,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>

#include <gspc/scheduler/job_states.hpp>

#define CHECK_SHOW(val, str)                                            \
  BOOST_AUTO_TEST_CASE (val)                                            \
  {                                                                     \
    BOOST_REQUIRE_EQUAL (gspc::scheduler::status::show (gspc::scheduler::status::val), str);  \
  }                                                                     \

CHECK_SHOW (PENDING,  "Scheduler::Pending")
CHECK_SHOW (RUNNING, "Scheduler::Running")
CHECK_SHOW (FINISHED, "Scheduler::Finished")
CHECK_SHOW (FAILED, "Scheduler::Failed")
CHECK_SHOW (CANCELED, "Scheduler::Canceled")
CHECK_SHOW (CANCELING, "Scheduler::Canceling")
