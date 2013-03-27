#define BOOST_TEST_MODULE UtilThreadEventTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/bool.hpp>
#include <fhg/util/bool_io.hpp>

#include <sstream>
#include <iostream>

BOOST_AUTO_TEST_CASE ( test_output_0 )
{
  using namespace fhg::util;

  std::ostringstream oss;

  bool_t b (false);
  oss << b;

  std::string actual = oss.str ();
  std::string expected = "0";

  BOOST_CHECK_EQUAL (expected, actual);
}

BOOST_AUTO_TEST_CASE ( test_output_false )
{
  using namespace fhg::util;

  std::ostringstream oss;
  oss << std::boolalpha;

  bool_t b (false);
  oss << b;

  std::string actual = oss.str ();
  std::string expected = "false";

  BOOST_CHECK_EQUAL (expected, actual);
}

BOOST_AUTO_TEST_CASE ( test_output_1 )
{
  using namespace fhg::util;

  std::ostringstream oss;

  bool_t b (true);
  oss << b;

  std::string actual = oss.str ();
  std::string expected = "1";

  BOOST_CHECK_EQUAL (expected, actual);
}

BOOST_AUTO_TEST_CASE ( test_output_true )
{
  using namespace fhg::util;

  std::ostringstream oss;
  oss << std::boolalpha;

  bool_t b (true);
  oss << b;

  std::string actual = oss.str ();
  std::string expected = "true";

  BOOST_CHECK_EQUAL (expected, actual);
}

BOOST_AUTO_TEST_CASE ( test_input_0 )
{
  using namespace fhg::util;

  std::istringstream iss ("0");
  bool_t b;
  iss >> b;

  BOOST_CHECK_EQUAL (false, b);
}

BOOST_AUTO_TEST_CASE ( test_input_false )
{
  using namespace fhg::util;

  std::istringstream iss ("false");
  bool_t b;
  iss >> b;

  BOOST_CHECK_EQUAL (false, b);
}

BOOST_AUTO_TEST_CASE ( test_input_1 )
{
  using namespace fhg::util;

  std::istringstream iss ("1");
  bool_t b;
  iss >> b;

  BOOST_CHECK_EQUAL (true, b);
}

BOOST_AUTO_TEST_CASE ( test_input_true )
{
  using namespace fhg::util;

  std::istringstream iss ("true");
  bool_t b;
  iss >> b;

  BOOST_CHECK_EQUAL (true, b);
}

BOOST_AUTO_TEST_CASE ( test_input_invalid )
{
  using namespace fhg::util;

  std::istringstream iss ("invalidinput");
  bool_t b;

  try
  {
    iss >> b;
    BOOST_ERROR ("invalid input must throw");
  }
  catch (...)
  {
    // everything fine
  }
}
