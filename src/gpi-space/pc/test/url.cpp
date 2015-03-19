#define BOOST_TEST_MODULE URLTest
#include <boost/test/unit_test.hpp>

#include <gpi-space/pc/url.hpp>
#include <gpi-space/pc/url_io.hpp>

#include <fhg/util/parse/error.hpp>

#include <util-generic/testing/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/printer/optional.hpp>
#include <fhg/util/boost/test/require_exception.hpp>

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

BOOST_AUTO_TEST_CASE (test_invalid_type)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("://foo"); }
    , "PARSE ERROR [0]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      " ://foo\n"
      "^\n"
    );
}

BOOST_AUTO_TEST_CASE (test_empty_param)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("file://bar?=baz"); }
    , "PARSE ERROR [11]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      "file://bar? =baz\n"
      "           ^\n"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_path)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://&"); }
    , "PARSE ERROR [12]: expected 'host {identifier_with_dot_and_space|ip}'\n"
      "protocoll:// &\n"
      "            ^\n"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_non_empty_path)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://path&"); }
    , "PARSE ERROR [16]: expected '?'\n"
      "protocoll://path &\n"
      "                ^\n"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_longer_path)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://longer/path&"); }
    , "PARSE ERROR [23]: expected '?'\n"
      "protocoll://longer/path &\n"
      "                       ^\n"
    );
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
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("pr:tocoll://"); }
    , "PARSE ERROR [3]: expected '//'\n"
      "pr: tocoll://\n"
      "   ^\n"
    );
}

BOOST_AUTO_TEST_CASE (type_with_colon_slash)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("pr:/ocoll://"); }
    , "PARSE ERROR [4]: expected '/'\n"
      "pr:/ ocoll://\n"
      "    ^\n"
    );
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
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://p/ath?"); }
    , "PARSE ERROR [18]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      "protocoll://p/ath? \n"
      "                  ^\n"
    );
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
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://p/ath?:=v"); }
    , "PARSE ERROR [18]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      "protocoll://p/ath? :=v\n"
      "                  ^\n"
    );
}

BOOST_AUTO_TEST_CASE (key_contains_colon_and_slashes)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://p/ath?://=v"); }
    , "PARSE ERROR [18]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      "protocoll://p/ath? ://=v\n"
      "                  ^\n"
    );
}

BOOST_AUTO_TEST_CASE (value_contains_colon_and_slashes)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://p/ath?k=://"); }
    , "PARSE ERROR [20]: expected 'value [a-zA-Z_0-9]+'\n"
      "protocoll://p/ath?k= ://\n"
      "                    ^\n"
    );
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
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://p/ath?k1=a&k?=b"); }
    , "PARSE ERROR [24]: expected '='\n"
      "protocoll://p/ath?k1=a&k ?=b\n"
      "                        ^\n"
    );
}

BOOST_AUTO_TEST_CASE (value_contains_questionmark)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://p/ath?k1=a&k2=?"); }
    , "PARSE ERROR [26]: expected 'value [a-zA-Z_0-9]+'\n"
      "protocoll://p/ath?k1=a&k2= ?\n"
      "                          ^\n"
    );
}

BOOST_AUTO_TEST_CASE (parameter_list_last_duplicate_throws)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::generic>
    ( [] { gpi::pc::url_t ("protocoll://p/ath?k=a&k=b"); }
    , "PARSE ERROR [25]: duplicate key 'k'\n"
      "protocoll://p/ath?k=a&k=b \n"
      "                         ^\n"
    );
}

BOOST_AUTO_TEST_CASE (empty_value)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://path?key="); }
    , "PARSE ERROR [21]: expected 'value [a-zA-Z_0-9]+'\n"
      "protocoll://path?key= \n"
      "                     ^\n"
    );
}

BOOST_AUTO_TEST_CASE (many_empty_value)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://path?k1=&k2="); }
    , "PARSE ERROR [20]: expected 'value [a-zA-Z_0-9]+'\n"
      "protocoll://path?k1= &k2=\n"
      "                    ^\n"
    );
}

BOOST_AUTO_TEST_CASE (port_number)
{
  gpi::pc::url_t const url ("protocoll://host:123");
  BOOST_REQUIRE_EQUAL (url.type (), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path (), "host:123");
}

BOOST_AUTO_TEST_CASE (port_missing)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://host:"); }
    , "PARSE ERROR [17]: expected 'port {UINT16|*}'\n"
      "protocoll://host: \n"
      "                 ^\n"
    );
}

BOOST_AUTO_TEST_CASE (port_without_host)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://:*"); }
    , "PARSE ERROR [12]: expected 'host {identifier_with_dot_and_space|ip}'\n"
      "protocoll:// :*\n"
      "            ^\n"
    );
}

BOOST_AUTO_TEST_CASE (port_to_large)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://host:65536"); }
    , "PARSE ERROR [22]: expected 'number in [0,65536)'\n"
      "protocoll://host:65536 \n"
      "                      ^\n"
    );

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
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://256"); }
    , "PARSE ERROR [15]: expected 'number in [0,256)'\n"
      "protocoll://256 \n"
      "               ^\n"
    );
}

BOOST_AUTO_TEST_CASE (host_ip_missing_first_dot)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://255"); }
    , "PARSE ERROR [15]: expected '.'\n"
      "protocoll://255 \n"
      "               ^\n"
    );

  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://255:255"); }
    , "PARSE ERROR [15]: expected '.'\n"
      "protocoll://255 :255\n"
      "               ^\n"
    );
}

BOOST_AUTO_TEST_CASE (host_ip_missing_second_dot)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://255.255"); }
    , "PARSE ERROR [19]: expected '.'\n"
      "protocoll://255.255 \n"
      "                   ^\n"
    );
}

BOOST_AUTO_TEST_CASE (host_ip_missing_third_dot)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( [] { gpi::pc::url_t ("protocoll://255.255.255"); }
    , "PARSE ERROR [23]: expected '.'\n"
      "protocoll://255.255.255 \n"
      "                       ^\n"
    );
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
