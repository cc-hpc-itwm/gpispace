// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE ini_parse
#include <boost/test/unit_test.hpp>

#include <fhg/util/parse/ini.hpp>
#include <fhg/util/parse/error.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>
#include <boost/utility.hpp>

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

BOOST_AUTO_TEST_CASE (empty_section_label)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, ".foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (section_with_sublabel_only_is_impossible)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[\"s\"]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "\"s\".foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (empty_section_ignored)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[][]\nfoo=bar"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, ".foo");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "bar");
}

BOOST_AUTO_TEST_CASE (multiple_section)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]\nk=v\n[2]\nk=v"));

  BOOST_REQUIRE_EQUAL (l.size(), 2);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
  BOOST_REQUIRE_EQUAL (boost::next (l.begin())->first, "2.k");
  BOOST_REQUIRE_EQUAL (boost::next (l.begin())->second, "v");
}

BOOST_AUTO_TEST_CASE (multiple_section_no_linebreak)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]k=v\n[2]k=v"));

  BOOST_REQUIRE_EQUAL (l.size(), 2);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
  BOOST_REQUIRE_EQUAL (boost::next (l.begin())->first, "2.k");
  BOOST_REQUIRE_EQUAL (boost::next (l.begin())->second, "v");
}

BOOST_AUTO_TEST_CASE (value_is_extended_to_endofline)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]k=v[2]k=v"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v[2]k=v");
}

BOOST_AUTO_TEST_CASE (quoted_value_is_not_extended_to_endofline_and_unquoted)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]k=\"v\"[2]k=v"));

  BOOST_REQUIRE_EQUAL (l.size(), 2);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
  BOOST_REQUIRE_EQUAL (boost::next (l.begin())->first, "2.k");
  BOOST_REQUIRE_EQUAL (boost::next (l.begin())->second, "v");
}

BOOST_AUTO_TEST_CASE (key_can_contain_comment_character_1)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]k#=v"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k#");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (key_can_contain_comment_character_2)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]k;=v"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k;");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (value_can_contain_comment_character_1)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]k=v#"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v#");
}

BOOST_AUTO_TEST_CASE (value_can_contain_comment_character_2)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]k=v;"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v;");
}

BOOST_AUTO_TEST_CASE (label_can_contain_comment_character_1)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1#]k=v"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1#.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (label_can_contain_comment_character_2)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1;]k=v"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1;.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (comment_can_follow_whitespace)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("[1]k=v\n# d=x"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (multiple_comment_lines)
{
  std::list<std::pair<std::string, std::string> > const l
    (fhg::util::parse::ini_from_string ("#[0] a=b\n[1]k=v\n# d=x\n; d=b"));

  BOOST_REQUIRE_EQUAL (l.size(), 1);
  BOOST_REQUIRE_EQUAL (l.begin()->first, "1.k");
  BOOST_REQUIRE_EQUAL (l.begin()->second, "v");
}
