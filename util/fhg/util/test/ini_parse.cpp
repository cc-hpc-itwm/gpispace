// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE ini_parse
#include <boost/test/unit_test.hpp>

#include <fhg/util/parse/ini.hpp>
#include <fhg/util/parse/error.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>

BOOST_AUTO_TEST_CASE (no_section)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&fhg::util::parse::ini_from_string, "foo = bar")
    , "PARSE ERROR [0]: expected '['\n foo = bar\n^\n"
    );
}

BOOST_AUTO_TEST_CASE (parse_key_value_with_spaces)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[default]\nfoo = bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "default.foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (parse_key_value_without_spaces)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[default]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "default.foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (parse_section_key_value_with_space)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[ test   ]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "test.foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (parse_section_with_empty_id)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[test \"\"]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "test..foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (parse_section_with_id)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[test \"id\"]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "test.id.foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (parse_section_with_dot_id)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[test \"id.1\"]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "test.id.1.foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (parse_section_with_multiple_dot_id_1)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[test \"id.1.2.3.4\"]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "test.id.1.2.3.4.foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (parse_section_with_multiple_dot_id_2)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[test \"id.1.2.3.4.5\"]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "test.id.1.2.3.4.5.foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (parse_comment)
{
  BOOST_REQUIRE_EQUAL
    (fhg::util::parse::ini_from_string ("; comment 1\n# comment 2").size(), 0);
}

BOOST_AUTO_TEST_CASE (parse_invalid_line)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&fhg::util::parse::ini_from_string, "illegal line here")
    , "PARSE ERROR [0]: expected '['\n illegal line here\n^\n"
    );
}
