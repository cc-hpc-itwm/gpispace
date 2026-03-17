// Copyright (C) 2013,2015,2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/iml/vmem/tmmgr.hpp>

#include <gspc/testing/iml/random/AllocationHandle.hpp>

#include <gspc/testing/flatten_nested_exceptions.hpp>
#include <gspc/testing/printer/set.hpp>
#include <gspc/testing/random.hpp>
#include <gspc/testing/require_maximum_running_time.hpp>

#include <boost/test/unit_test.hpp>

#include <boost/test/data/test_case.hpp>

#include <chrono>
#include <set>

namespace
{
  static std::size_t num_handles (1 << 18);

  std::set<gspc::iml::AllocationHandle> handles_random()
  {
    std::set<gspc::iml::AllocationHandle> handles;

    while (handles.size() < num_handles)
    {
      handles.emplace (gspc::testing::random<gspc::iml::AllocationHandle>{}());
    }

    return handles;
  }

  std::set<gspc::iml::AllocationHandle> handles_sequence()
  {
    std::set<gspc::iml::AllocationHandle> handles;

    for (std::size_t h (0); h < num_handles; ++h)
    {
      handles.emplace (h);
    }

    return handles;
  }

  std::vector<std::set<gspc::iml::AllocationHandle>> handless()
  {
    return {handles_random(), handles_sequence()};
  }
}

BOOST_DATA_TEST_CASE (fill_clear, handless(), handles)
{
  iml_client::vmem::tmmgr tmmgr (num_handles, 1);

  auto const start (std::chrono::steady_clock::now());

  GSPC_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
  {
    for (gspc::iml::AllocationHandle const& handle : handles)
    {
      tmmgr.alloc (handle, 1);
    }
  };

  GSPC_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME
    ( std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now() - start)
    )
  {
    for (gspc::iml::AllocationHandle const& handle : handles)
    {
      tmmgr.free (handle);
    }
  };
}
