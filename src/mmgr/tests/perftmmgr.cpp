// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE mmgr_perftmmgr
#include <boost/test/unit_test.hpp>

#include <mmgr/tmmgr.h>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/now.hpp>

#include <set>

namespace
{
  static std::size_t num_handles (1 << 18);

  std::set<Handle_t> handles_random()
  {
    std::set<Handle_t> handles;

    srand (314159);

    while (handles.size() < num_handles)
    {
      handles.insert (rand());
    }

    return handles;
  }

  std::set<Handle_t> handles_sequence()
  {
    std::set<Handle_t> handles;

    for (Handle_t h (0); h < num_handles; ++h)
    {
      handles.insert (h);
    }

    return handles;
  }

  void test_fill_clear (std::set<Handle_t> const& handles)
  {
    Tmmgr_t tmmgr = nullptr;

    tmmgr_init (&tmmgr, num_handles, 1);

    double t_alloc (-fhg::util::now());

    for (Handle_t const& handle : handles)
    {
      BOOST_REQUIRE_EQUAL (tmmgr_alloc (&tmmgr, handle, 1), ALLOC_SUCCESS);
    }

    t_alloc += fhg::util::now();

    BOOST_REQUIRE_LT (t_alloc, 1.);

    double t_free (-fhg::util::now());

    for (Handle_t const& handle : handles)
    {
      BOOST_REQUIRE_EQUAL (tmmgr_free (&tmmgr, handle), RET_SUCCESS);
    }

    t_free += fhg::util::now();

    BOOST_REQUIRE_LT (t_free, t_alloc);
  }
}

BOOST_AUTO_TEST_CASE (fill_clear)
{
  test_fill_clear (handles_random());
  test_fill_clear (handles_sequence());
}
