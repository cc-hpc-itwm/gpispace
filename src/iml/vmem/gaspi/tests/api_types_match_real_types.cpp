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

#include <iml/gaspi/NetdevID.hpp>
#include <iml/vmem/gaspi/types.hpp>

#include <util-generic/testing/require_type.hpp>

#include <boost/test/unit_test.hpp>

#include <GASPI.h>

namespace gpi
{
  // We don't want to expose GASPI.h to users, but we still want the
  // correct types, so these checks assert that the public definition
  // and GASPI's definition are equal.
  BOOST_AUTO_TEST_CASE (api_types_match_real_types)
  {
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (offset_t, gaspi_offset_t);
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (rank_t, gaspi_rank_t);
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (queue_desc_t, gaspi_queue_id_t);
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (size_t, gaspi_size_t);
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (notification_t, gaspi_notification_t);
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
      (notification_id_t, gaspi_notification_id_t);
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (timeout_t, gaspi_timeout_t);
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL (segment_id_t, gaspi_segment_id_t);
    FHG_UTIL_TESTING_CHECK_TYPE_EQUAL
      (decltype (iml::gaspi::NetdevID::value), gaspi_int);
  }
}
