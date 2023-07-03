// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
