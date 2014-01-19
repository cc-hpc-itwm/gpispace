// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_big
#include <boost/test/unit_test.hpp>

#include <process.hpp>

namespace
{
  struct scoped_allocation
  {
    scoped_allocation (std::size_t size)
      : _ptr (new int[size])
    {}
    ~scoped_allocation()
    {
      delete[] _ptr;
    }
    operator int* ()
    {
      return _ptr;
    }
  private:
    int* _ptr;
  };
}

BOOST_AUTO_TEST_CASE (process_big)
{
  BOOST_REQUIRE_GT (boost::unit_test::framework::master_test_suite().argc, 1);

  const std::size_t size
    (atoi (boost::unit_test::framework::master_test_suite().argv[1]));
  const std::size_t count (size / sizeof (int));

  scoped_allocation in (count);
  scoped_allocation out (count);

  for (std::size_t i (0); i < count; ++i)
  {
    in[i] = i;
    out[i] = 0;
  }

  {
    const std::size_t ret (process::execute ("cat", in, size, out, size));

    BOOST_REQUIRE_EQUAL (ret, size);

    for (std::size_t i (0); i < count; ++i)
    {
      BOOST_REQUIRE_EQUAL (in[i], out[i]);
    }
  }

  // inplace
  {
    const std::size_t ret (process::execute ("cat", in, size, in, size));

    BOOST_REQUIRE_EQUAL (ret, size);

    for (std::size_t i (0); i < count; ++i)
    {
      BOOST_REQUIRE_EQUAL (in[i], out[i]);
    }
  }
}
