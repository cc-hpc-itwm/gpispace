#include <boost/test/unit_test.hpp>

#include <gpi-space/pc/url.hpp>
#include <gpi-space/pc/url_io.hpp>

#include <fhg/util/parse/error.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <util-generic/testing/printer/optional.hpp>
#include <util-generic/testing/require_exception.hpp>

#include <iterator>

BOOST_AUTO_TEST_CASE (test_url_basics)
{
  gpi::pc::url_t url ("http", "localhost:8080");
  url.set ("foo", "bar");
  url.set ("bar", "baz");

  BOOST_CHECK_EQUAL (url.type (), "http");
  BOOST_CHECK_EQUAL (url.path (), "localhost:8080");
  BOOST_CHECK_EQUAL (url.get ("foo").get_value_or (""), "bar");
  BOOST_CHECK_EQUAL (url.get ("bar").get_value_or (""), "baz");
}

BOOST_AUTO_TEST_CASE (test_url_ctor)
{
  gpi::pc::url_t url ("http://localhost:8080?foo=bar&bar=baz");

  BOOST_CHECK_EQUAL (url.type (), "http");
  BOOST_CHECK_EQUAL (url.path (), "localhost:8080");
  BOOST_CHECK_EQUAL (url.get ("foo").get_value_or (""), "bar");
  BOOST_CHECK_EQUAL (url.get ("bar").get_value_or (""), "baz");
}

namespace
{
  fhg::util::parse::position_string make_position
    ( std::string const& input
    , std::size_t offset
    )
  {
    fhg::util::parse::position_string pos (input);
    pos.advance (offset);
    return pos;
  }

  void expect_generic_parse_error ( std::string input
                                  , std::size_t error_position
                                  , std::string message
                                  )
  {
    fhg::util::testing::require_exception
      ( [&input]
        {
          gpi::pc::url_t {input};
        }
      , fhg::util::parse::error::generic
          (message, make_position (input, error_position))
      );
  }
  void expect_mismatch_parse_error ( std::string input
                                   , std::size_t error_position
                                   , std::string expectation
                                   )
  {
    fhg::util::testing::require_exception
      ( [&input]
        {
          gpi::pc::url_t {input};
        }
      , fhg::util::parse::error::expected
          (expectation, make_position (input, error_position))
      );
  }
}

BOOST_AUTO_TEST_CASE (test_invalid_type)
{
  expect_mismatch_parse_error
    ("://foo", 0, "identifier [a-zA-Z_][a-zA-Z_0-9]*");
}

BOOST_AUTO_TEST_CASE (test_empty_param)
{
  expect_mismatch_parse_error
    ("file://bar?=baz", 11, "identifier [a-zA-Z_][a-zA-Z_0-9]*");
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_path)
{
  expect_mismatch_parse_error
    ("protocoll://&", 12, "host {identifier_with_dot_and_space|ip}");
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_non_empty_path)
{
  expect_mismatch_parse_error ("protocoll://path&", 16, "?");
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_longer_path)
{
  expect_mismatch_parse_error ("protocoll://longer/path&", 23, "?");
}

BOOST_AUTO_TEST_CASE (simple_type)
{
  gpi::pc::url_t const url ("protocoll://");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (type_with_colon)
{
  expect_mismatch_parse_error ("pr:tocoll://", 3, "//");
}

BOOST_AUTO_TEST_CASE (type_with_colon_slash)
{
  expect_mismatch_parse_error ("pr:/ocoll://", 4, "/");
}

BOOST_AUTO_TEST_CASE (no_colon_required)
{
  gpi::pc::url_t const url ("just_the_protocoll");

  BOOST_REQUIRE_EQUAL (url.type(), "just_the_protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (no_path_required)
{
  gpi::pc::url_t const url ("just_the_protocoll://");

  BOOST_REQUIRE_EQUAL (url.type(), "just_the_protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (empty_parameter_list)
{
  expect_mismatch_parse_error
    ("protocoll://p/ath?", 18, "identifier [a-zA-Z_][a-zA-Z_0-9]*");
}

BOOST_AUTO_TEST_CASE (parameter_list_single)
{
  gpi::pc::url_t const url ("protocoll://p/ath?k=v");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "p/ath");
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (key_contains_colon)
{
  expect_mismatch_parse_error
    ("protocoll://p/ath?:=v", 18, "identifier [a-zA-Z_][a-zA-Z_0-9]*");
}

BOOST_AUTO_TEST_CASE (key_contains_colon_and_slashes)
{
  expect_mismatch_parse_error
    ("protocoll://p/ath?://=v", 18, "identifier [a-zA-Z_][a-zA-Z_0-9]*");
}

BOOST_AUTO_TEST_CASE (value_contains_colon_and_slashes)
{
  expect_mismatch_parse_error
    ("protocoll://p/ath?k=://", 20, "value [a-zA-Z_0-9]+");
}

BOOST_AUTO_TEST_CASE (parameter_list_many)
{
  gpi::pc::url_t const url ("protocoll://p/ath?k1=a&k2=b");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "p/ath");
  BOOST_REQUIRE_EQUAL (url.args().size(), 2);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k1");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "a");
  BOOST_REQUIRE_EQUAL (std::next (url.args().begin())->first, "k2");
  BOOST_REQUIRE_EQUAL (std::next (url.args().begin())->second, "b");
}

BOOST_AUTO_TEST_CASE (key_contains_questionmark)
{
  expect_mismatch_parse_error ("protocoll://p/ath?k1=a&k?=b", 24, "=");
}

BOOST_AUTO_TEST_CASE (value_contains_questionmark)
{
  expect_mismatch_parse_error
    ("protocoll://p/ath?k1=a&k2=?", 26, "value [a-zA-Z_0-9]+");
}

BOOST_AUTO_TEST_CASE (parameter_list_last_duplicate_throws)
{
  expect_generic_parse_error
    ("protocoll://p/ath?k=a&k=b", 25, "duplicate key 'k'");
}

BOOST_AUTO_TEST_CASE (empty_value)
{
  expect_mismatch_parse_error
    ("protocoll://path?key=", 21, "value [a-zA-Z_0-9]+");
}

BOOST_AUTO_TEST_CASE (many_empty_value)
{
  expect_mismatch_parse_error
    ("protocoll://path?k1=&k2=", 20, "value [a-zA-Z_0-9]+");
}

BOOST_AUTO_TEST_CASE (port_number)
{
  gpi::pc::url_t const url ("protocoll://host:123");
  BOOST_REQUIRE_EQUAL (url.type (), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path (), "host:123");
}

BOOST_AUTO_TEST_CASE (port_missing)
{
  expect_mismatch_parse_error ("protocoll://host:", 17, "port {UINT16|*}");
}

BOOST_AUTO_TEST_CASE (port_without_host)
{
  expect_mismatch_parse_error
    ("protocoll://:*", 12, "host {identifier_with_dot_and_space|ip}");
}

BOOST_AUTO_TEST_CASE (port_to_large)
{
  expect_mismatch_parse_error
    ("protocoll://host:65536", 22, "number in [0,65536)");
}

BOOST_AUTO_TEST_CASE (port_star)
{
  gpi::pc::url_t const url ("protocoll://host:*");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host:*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (port_star_param_list)
{
  gpi::pc::url_t const url ("protocoll://host:*?k_ey=5");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host:*");
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k_ey");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "5");
}

BOOST_AUTO_TEST_CASE (host_name_with_dot)
{
  gpi::pc::url_t const url ("protocoll://host.domain");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host.domain");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_name_with_dots)
{
  gpi::pc::url_t const url ("protocoll://host.domain.that.is.somewhere");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host.domain.that.is.somewhere");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_name_with_dot_sequence)
{
  gpi::pc::url_t const url ("protocoll://host..domain..:*");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host..domain..:*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_ip)
{
  gpi::pc::url_t const url ("protocoll://127.0.0.1");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "127.0.0.1");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_ip_port_num)
{
  gpi::pc::url_t const url ("protocoll://127.0.0.1:1");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "127.0.0.1:1");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_ip_port_star)
{
  gpi::pc::url_t const url ("protocoll://127.0.0.1:*");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "127.0.0.1:*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_ip_number_to_large)
{
  expect_mismatch_parse_error ("protocoll://256", 15, "number in [0,256)");
}

BOOST_AUTO_TEST_CASE (host_ip_missing_first_dot)
{
  expect_mismatch_parse_error ("protocoll://255", 15, ".");
  expect_mismatch_parse_error ("protocoll://255:255", 15, ".");
}

BOOST_AUTO_TEST_CASE (host_ip_missing_second_dot)
{
  expect_mismatch_parse_error ("protocoll://255.255", 19, ".");
  expect_mismatch_parse_error ("protocoll://255.255:255", 19, ".");
}

BOOST_AUTO_TEST_CASE (host_ip_missing_third_dot)
{
  expect_mismatch_parse_error ("protocoll://255.255.255", 23, ".");
  expect_mismatch_parse_error ("protocoll://255.255.255:255", 23, ".");
}

BOOST_AUTO_TEST_CASE (host_star)
{
  gpi::pc::url_t const url ("protocoll://*");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_star_port_start)
{
  gpi::pc::url_t const url ("protocoll://*:*");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "*:*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_star_port_start_parameter)
{
  gpi::pc::url_t const url ("protocoll://*:*?k=v");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "*:*");
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (no_host_but_parameter)
{
  gpi::pc::url_t const url ("protocoll://?k=v");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (parse_file_url)
{
  {
    gpi::pc::url_t url ("file:///foo/bar/baz");
    BOOST_REQUIRE_EQUAL (url.type (), "file");
    BOOST_REQUIRE_EQUAL (url.path (), "/foo/bar/baz");
  }

  {
    gpi::pc::url_t url ("file://foo/bar/baz");
    BOOST_REQUIRE_EQUAL (url.type (), "file");
    BOOST_REQUIRE_EQUAL (url.path (), "foo/bar/baz");
  }
}

BOOST_AUTO_TEST_CASE (url_io)
{
  const std::string url_string ("file:///foo/bar/baz?a=b&c=d&d=e");

  {
    std::stringstream sstr;

    gpi::pc::url_t url_o (url_string);
    sstr << url_o;

    BOOST_REQUIRE_EQUAL (sstr.str (), url_string);

    gpi::pc::url_t url_i (sstr.str());

    BOOST_CHECK_EQUAL (url_i.type ()  , url_o.type ());
    BOOST_CHECK_EQUAL (url_i.path ()  , url_o.path ());
    BOOST_CHECK_EQUAL (url_i.get ("a"), url_o.get ("a"));
    BOOST_CHECK_EQUAL (url_i.get ("c"), url_o.get ("c"));
    BOOST_CHECK_EQUAL (url_i.get ("d"), url_o.get ("d"));

    std::stringstream sstr2;

    sstr2 << url_i;
    BOOST_REQUIRE_EQUAL (sstr2.str (), url_string);
  }
}
