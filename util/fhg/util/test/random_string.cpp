// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE random_string
#include <boost/test/unit_test.hpp>

#include <fhg/util/random_string.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>

#include <cmath>

BOOST_AUTO_TEST_CASE (string_of_empty)
{
  fhg::util::boost::test::require_exception<std::runtime_error>
    ( boost::bind (&fhg::util::random_string_of, std::string())
    , "random_string_of (empty_string)"
    );
}

BOOST_TEST_DONT_PRINT_LOG_VALUE (std::string::const_iterator)

BOOST_AUTO_TEST_CASE (string_of_one)
{
  std::string const chars ("c");

  BOOST_FOREACH (char const& c, fhg::util::random_string_of (chars))
  {
    BOOST_REQUIRE_NE (chars.end(), std::find (chars.begin(), chars.end(), c));
  }
}

BOOST_AUTO_TEST_CASE (string_of_some)
{
  std::string const chars ("abcdefgh0123");

  BOOST_FOREACH (char const& c, fhg::util::random_string_of (chars))
  {
    BOOST_REQUIRE_NE (chars.end(), std::find (chars.begin(), chars.end(), c));
  }
}

BOOST_AUTO_TEST_CASE (string_of_distribution)
{
  std::string const chars ("011");
  std::size_t count[2] = {0,0};
  std::size_t len (0);

  while (count[0] == 0 || len < (1UL << 20))
  {
    std::string const s (fhg::util::random_string_of (chars));

    len += s.size();

    BOOST_FOREACH (char const& c, s)
    {
      ++count[c - '0'];
    }
  }

  double const ratio (double (count[1]) / double (count[0]));

  BOOST_REQUIRE_LT (std::fabs (ratio - 2.0), 0.1);
}

BOOST_AUTO_TEST_CASE (string_without)
{
  std::string const except
    (fhg::util::random_string_of ("0123456789abcdefghijklmnopqrstuvwyxz${}"));

  BOOST_FOREACH (char const& c, fhg::util::random_string_without (except))
  {
    BOOST_REQUIRE_EQUAL
      (except.end(), std::find (except.begin(), except.end(), c));
  }
}
