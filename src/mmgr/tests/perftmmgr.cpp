#include <mmgr/tmmgr.hpp>

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

  std::set<gspc::vmem::Handle_t> handles_random()
  {
    std::set<gspc::vmem::Handle_t> handles;

    while (handles.size() < num_handles)
    {
      handles.emplace (fhg::util::testing::random<gspc::vmem::Handle_t>{}());
    }

    return handles;
  }

  std::set<gspc::vmem::Handle_t> handles_sequence()
  {
    std::set<gspc::vmem::Handle_t> handles;

    for (gspc::vmem::Handle_t h (0); h < num_handles; ++h)
    {
      handles.insert (h);
    }

    return handles;
  }

  std::vector<std::set<gspc::vmem::Handle_t>> handless()
  {
    return {handles_random(), handles_sequence()};
  }
}

BOOST_DATA_TEST_CASE (fill_clear, handless(), handles)
{
  gspc::vmem::tmmgr tmmgr (num_handles, 1);

  auto const start (std::chrono::steady_clock::now());

  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME (std::chrono::seconds (1))
  {
    for (gspc::vmem::Handle_t const& handle : handles)
    {
      tmmgr.alloc (handle, 1);
    }
  };

  FHG_UTIL_TESTING_REQUIRE_MAXIMUM_RUNNING_TIME
    ( std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now() - start)
    )
  {
    for (gspc::vmem::Handle_t const& handle : handles)
    {
      tmmgr.free (handle);
    }
  };
}
