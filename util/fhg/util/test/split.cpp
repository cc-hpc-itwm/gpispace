// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE split
#include <boost/test/unit_test.hpp>

#include <fhg/util/split.hpp>

#include <vector>
#include <string>

namespace
{
  std::vector<std::string> split_at_dot (std::string path)
  {
    std::vector<std::string> result;
    fhg::util::split (path, ".", std::back_inserter (result));
    return result;
  }
}

BOOST_AUTO_TEST_CASE (empty_string_results_in_empty_set)
{
  const std::vector<std::string> path (split_at_dot (std::string()));

  const std::vector<std::string> expected;

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (path.begin(), path.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE (trailing_empty_element_is_ignored)
{
  const std::vector<std::string> path (split_at_dot ("foo."));

  std::vector<std::string> expected;
  expected.push_back("foo");

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (path.begin(), path.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE (non_trailing_empty_element_is_preserved)
{
  const std::vector<std::string> path (split_at_dot ("."));

  std::vector<std::string> expected;
  expected.push_back(std::string());

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (path.begin(), path.end(), expected.begin(), expected.end());
}

BOOST_AUTO_TEST_CASE (non_empty_elements_are_preserved)
{
  const std::vector<std::string> path (split_at_dot ("fhg.log.logger.1"));

  std::vector<std::string> expected;
  expected.push_back("fhg");
  expected.push_back("log");
  expected.push_back("logger");
  expected.push_back("1");

  BOOST_REQUIRE_EQUAL_COLLECTIONS
    (path.begin(), path.end(), expected.begin(), expected.end());
}
