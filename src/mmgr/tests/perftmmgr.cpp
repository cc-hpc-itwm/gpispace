#include <boost/test/unit_test.hpp>

#include <mmgr/tmmgr.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_maximum_running_time.hpp>

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

  void test_fill_clear (std::set<gspc::vmem::Handle_t> const& handles)
  {
    gspc::vmem::tmmgr tmmgr (num_handles, 1);

    std::chrono::steady_clock::time_point const start
      (std::chrono::steady_clock::now());

    {
      fhg::util::testing::require_maximum_running_time<std::chrono::seconds>
        const max_time_for_alloc (1);

      for (gspc::vmem::Handle_t const& handle : handles)
      {
        tmmgr.alloc (handle, 1);
      }
    }

    {
      fhg::util::testing::require_maximum_running_time
        <std::chrono::milliseconds>
          const max_time_for_free
            (std::chrono::duration_cast<std::chrono::milliseconds>
              (std::chrono::steady_clock::now() - start)
            );

      for (gspc::vmem::Handle_t const& handle : handles)
      {
        tmmgr.free (handle);
      }
    }
  }
}

BOOST_AUTO_TEST_CASE (fill_clear)
{
  test_fill_clear (handles_random());
  test_fill_clear (handles_sequence());
}
