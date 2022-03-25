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
