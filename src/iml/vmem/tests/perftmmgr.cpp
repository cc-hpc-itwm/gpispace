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

#include <iml/vmem/tmmgr.hpp>

#include <iml/testing/random/AllocationHandle.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/set.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

#include <boost/test/data/test_case.hpp>
#include <boost/test/unit_test.hpp>

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
