// Copyright (C) 2015-2016,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/chrono_unit_for_period.hpp>
#include <gspc/util/ostream/put_time.hpp>
#include <gspc/testing/printer/generic.hpp>

#include <chrono>

GSPC_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  ( <typename Rep BOOST_PP_COMMA() typename Period>
  , std::chrono::duration<Rep BOOST_PP_COMMA() Period>
  , os
  , duration
  )
{
  os << GSPC_BOOST_TEST_PRINT_LOG_VALUE_HELPER (duration.count()) << " "
     << gspc::util::chrono_unit_for_period<Period>();
}

GSPC_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  ( <typename Clock BOOST_PP_COMMA() typename Duration>
  , std::chrono::time_point<Clock BOOST_PP_COMMA() Duration>
  , os
  , time_point
  )
{
  os << gspc::util::ostream::put_time<Clock, Duration> (time_point);
}
