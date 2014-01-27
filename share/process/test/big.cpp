// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_big
#include <boost/test/unit_test.hpp>

#include <process.hpp>

BOOST_AUTO_TEST_CASE (process_big)
{
  BOOST_REQUIRE_GT (boost::unit_test::framework::master_test_suite().argc, 1);

  const std::size_t size
    (atoi (boost::unit_test::framework::master_test_suite().argv[1]));
  const std::size_t count (size / sizeof (int));

  std::vector<int> in (count);
  std::vector<int> out (count);

  for (std::size_t i (0); i < count; ++i)
  {
    in[i] = i;
    out[i] = 0;
  }

  {
    BOOST_REQUIRE_EQUAL (process::execute ("cat", &in[0], size, &out[0], size), size);

    for (std::size_t i (0); i < count; ++i)
    {
      BOOST_REQUIRE_EQUAL (in[i], out[i]);
    }
  }

  // inplace
  {
    BOOST_REQUIRE_EQUAL (process::execute ("cat", &in[0], size, &in[0], size), size);

    for (std::size_t i (0); i < count; ++i)
    {
      BOOST_REQUIRE_EQUAL (in[i], out[i]);
    }
  }
}
