// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE path
#include <boost/test/unit_test.hpp>

#include <fhglog/logger_path.hpp>
#include <iostream>

BOOST_AUTO_TEST_CASE (to_string)
{
  BOOST_REQUIRE_EQUAL
    (fhg::log::logger_path ("fhg.logger.test").str(), "fhg.logger.test");
  BOOST_REQUIRE_EQUAL
    (fhg::log::logger_path ("fhg.logger.test"), std::string ("fhg.logger.test"));
}

BOOST_AUTO_TEST_CASE (operator_slash)
{
  BOOST_REQUIRE_EQUAL
    (fhg::log::logger_path ("fhg") / "test" / 1, std::string ("fhg.test.1"));
}

BOOST_AUTO_TEST_CASE (iostreaming)
{
  const fhg::log::logger_path p ("fhg.logger.test");

  std::stringstream sstr;
  sstr << p;

  fhg::log::logger_path p_in;
  sstr >> p_in;

  BOOST_REQUIRE_EQUAL (p, p_in);
}
