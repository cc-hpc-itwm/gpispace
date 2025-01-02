// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/vmem/tmmgr.hpp>

#include <iml/testing/random/AllocationHandle.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <chrono>
#include <set>

namespace
{
  static std::size_t num_handles (1 << 18);

  std::set<iml::AllocationHandle> handles_random()
  {
    std::set<iml::AllocationHandle> handles;

    while (handles.size() < num_handles)
    {
      handles.emplace (fhg::util::testing::random<iml::AllocationHandle>{}());
    }

    return handles;
  }

  std::set<iml::AllocationHandle> handles_sequence()
  {
    std::set<iml::AllocationHandle> handles;

    for (std::size_t h (0); h < num_handles; ++h)
    {
      handles.emplace (h);
    }

    return handles;
  }

  std::vector<std::set<iml::AllocationHandle>> handless()
  {
    return {handles_random(), handles_sequence()};
  }
}

BOOST_DATA_TEST_CASE (fill_clear, handless(), handles)
{
  iml_client::vmem::tmmgr tmmgr (num_handles, 1);

  auto const start (std::chrono::steady_clock::now());

  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
  {
    for (iml::AllocationHandle const& handle : handles)
    {
      tmmgr.alloc (handle, 1);
    }
  };

  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME
    ( std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now() - start)
    )
  {
    for (iml::AllocationHandle const& handle : handles)
    {
      tmmgr.free (handle);
    }
  };
}
