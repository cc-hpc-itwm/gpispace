// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <util-generic/chrono_unit_for_period.hpp>
#include <util-generic/ostream/put_time.hpp>
#include <util-generic/testing/printer/generic.hpp>

#include <chrono>

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  ( <typename Rep BOOST_PP_COMMA() typename Period>
  , std::chrono::duration<Rep BOOST_PP_COMMA() Period>
  , os
  , duration
  )
{
  os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (duration.count()) << " "
     << fhg::util::chrono_unit_for_period<Period>();
}

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  ( <typename Clock BOOST_PP_COMMA() typename Duration>
  , std::chrono::time_point<Clock BOOST_PP_COMMA() Duration>
  , os
  , time_point
  )
{
  os << fhg::util::ostream::put_time<Clock, Duration> (time_point);
}
