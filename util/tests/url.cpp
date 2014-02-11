#define BOOST_TEST_MODULE URLTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/url.hpp>
#include <fhg/util/url_io.hpp>

#include <fhg/util/parse/error.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>
#include <boost/utility.hpp>

BOOST_AUTO_TEST_CASE (test_url_basics)
{
  fhg::util::url_t url ("http", "localhost:8080");
  url.set ("foo", "bar");
  url.set ("bar", "baz");

  BOOST_CHECK_EQUAL (url.type (), "http");
  BOOST_CHECK_EQUAL (url.path (), "localhost:8080");
  BOOST_CHECK_EQUAL (url.get ("foo").get_value_or (""), "bar");
  BOOST_CHECK_EQUAL (url.get ("bar").get_value_or (""), "baz");
}

BOOST_AUTO_TEST_CASE (test_url_ctor)
{
  fhg::util::url_t url ("http://localhost:8080?foo=bar&bar=baz");

  BOOST_CHECK_EQUAL (url.type (), "http");
  BOOST_CHECK_EQUAL (url.path (), "localhost:8080");
  BOOST_CHECK_EQUAL (url.get ("foo").get_value_or (""), "bar");
  BOOST_CHECK_EQUAL (url.get ("bar").get_value_or (""), "baz");
}

namespace
{
  fhg::util::url_t ctor (std::string const& u)
  {
    return fhg::util::url_t (u);
  }
}

BOOST_AUTO_TEST_CASE (test_invalid_type)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "://foo")
    , "PARSE ERROR [0]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      " ://foo\n"
      "^\n"
    );
}

BOOST_AUTO_TEST_CASE (test_empty_param)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "file://bar?=baz")
    , "PARSE ERROR [11]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      "file://bar? =baz\n"
      "           ^\n"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_path)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://&")
    , "PARSE ERROR [12]: expected 'host {identifier_with_dot|ip}'\n"
      "protocoll:// &\n"
      "            ^\n"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_non_empty_path)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://path&")
    , "PARSE ERROR [16]: expected '?'\n"
      "protocoll://path &\n"
      "                ^\n"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_longer_path)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://longer/path&")
    , "PARSE ERROR [23]: expected '?'\n"
      "protocoll://longer/path &\n"
      "                       ^\n"
    );
}

BOOST_AUTO_TEST_CASE (simple_type)
{
  fhg::util::url_t const url ("protocoll://");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (type_with_colon)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "pr:tocoll://")
    , "PARSE ERROR [3]: expected '//'\n"
      "pr: tocoll://\n"
      "   ^\n"
    );
}

BOOST_AUTO_TEST_CASE (type_with_colon_slash)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "pr:/ocoll://")
    , "PARSE ERROR [4]: expected '/'\n"
      "pr:/ ocoll://\n"
      "    ^\n"
    );
}

BOOST_AUTO_TEST_CASE (no_colon_required)
{
  fhg::util::url_t const url ("just_the_protocoll");

  BOOST_REQUIRE_EQUAL (url.type(), "just_the_protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (no_path_required)
{
  fhg::util::url_t const url ("just_the_protocoll://");

  BOOST_REQUIRE_EQUAL (url.type(), "just_the_protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (empty_parameter_list)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://p/ath?")
    , "PARSE ERROR [18]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      "protocoll://p/ath? \n"
      "                  ^\n"
    );
}

BOOST_AUTO_TEST_CASE (parameter_list_single)
{
  fhg::util::url_t const url ("protocoll://p/ath?k=v");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "p/ath");
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (key_contains_colon)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://p/ath?:=v")
    , "PARSE ERROR [18]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      "protocoll://p/ath? :=v\n"
      "                  ^\n"
    );
}

BOOST_AUTO_TEST_CASE (key_contains_colon_and_slashes)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://p/ath?://=v")
    , "PARSE ERROR [18]: expected 'identifier [a-zA-Z_][a-zA-Z_0-9]*'\n"
      "protocoll://p/ath? ://=v\n"
      "                  ^\n"
    );
}

BOOST_AUTO_TEST_CASE (value_contains_colon_and_slashes)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://p/ath?k=://")
    , "PARSE ERROR [20]: expected 'value [a-zA-Z_0-9]+'\n"
      "protocoll://p/ath?k= ://\n"
      "                    ^\n"
    );
}

BOOST_AUTO_TEST_CASE (parameter_list_many)
{
  fhg::util::url_t const url ("protocoll://p/ath?k1=a&k2=b");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "p/ath");
  BOOST_REQUIRE_EQUAL (url.args().size(), 2);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k1");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "a");
  BOOST_REQUIRE_EQUAL (boost::next (url.args().begin())->first, "k2");
  BOOST_REQUIRE_EQUAL (boost::next (url.args().begin())->second, "b");
}

BOOST_AUTO_TEST_CASE (key_contains_questionmark)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://p/ath?k1=a&k?=b")
    , "PARSE ERROR [24]: expected '='\n"
      "protocoll://p/ath?k1=a&k ?=b\n"
      "                        ^\n"
    );
}

BOOST_AUTO_TEST_CASE (value_contains_questionmark)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://p/ath?k1=a&k2=?")
    , "PARSE ERROR [26]: expected 'value [a-zA-Z_0-9]+'\n"
      "protocoll://p/ath?k1=a&k2= ?\n"
      "                          ^\n"
    );
}

BOOST_AUTO_TEST_CASE (parameter_list_last_duplicate_throws)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::generic>
    ( boost::bind (&ctor, "protocoll://p/ath?k=a&k=b")
    , "PARSE ERROR [25]: duplicate key 'k'\n"
      "protocoll://p/ath?k=a&k=b \n"
      "                         ^\n"
    );
}

BOOST_AUTO_TEST_CASE (empty_value)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://path?key=")
    , "PARSE ERROR [21]: expected 'value [a-zA-Z_0-9]+'\n"
      "protocoll://path?key= \n"
      "                     ^\n"
    );
}

BOOST_AUTO_TEST_CASE (many_empty_value)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://path?k1=&k2=")
    , "PARSE ERROR [20]: expected 'value [a-zA-Z_0-9]+'\n"
      "protocoll://path?k1= &k2=\n"
      "                    ^\n"
    );
}

BOOST_AUTO_TEST_CASE (port_number)
{
  fhg::util::url_t const url ("protocoll://host:123");
  BOOST_REQUIRE_EQUAL (url.type (), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path (), "host:123");
}

BOOST_AUTO_TEST_CASE (port_missing)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://host:")
    , "PARSE ERROR [17]: expected 'port {UINT16|*}'\n"
      "protocoll://host: \n"
      "                 ^\n"
    );
}

BOOST_AUTO_TEST_CASE (port_without_host)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://:*")
    , "PARSE ERROR [12]: expected 'host {identifier_with_dot|ip}'\n"
      "protocoll:// :*\n"
      "            ^\n"
    );
}

BOOST_AUTO_TEST_CASE (port_to_large)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://host:65536")
    , "PARSE ERROR [22]: expected 'number in [0,65536)'\n"
      "protocoll://host:65536 \n"
      "                      ^\n"
    );

}

BOOST_AUTO_TEST_CASE (port_star)
{
  fhg::util::url_t const url ("protocoll://host:*");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host:*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (port_star_param_list)
{
  fhg::util::url_t const url ("protocoll://host:*?k_ey=5");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host:*");
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k_ey");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "5");
}

BOOST_AUTO_TEST_CASE (host_name_with_dot)
{
  fhg::util::url_t const url ("protocoll://host.domain");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host.domain");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_name_with_dots)
{
  fhg::util::url_t const url ("protocoll://host.domain.that.is.somewhere");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host.domain.that.is.somewhere");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_name_with_dot_sequence)
{
  fhg::util::url_t const url ("protocoll://host..domain..:*");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "host..domain..:*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_ip)
{
  fhg::util::url_t const url ("protocoll://127.0.0.1");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "127.0.0.1");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_ip_port_num)
{
  fhg::util::url_t const url ("protocoll://127.0.0.1:1");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "127.0.0.1:1");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_ip_port_star)
{
  fhg::util::url_t const url ("protocoll://127.0.0.1:*");
  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "127.0.0.1:*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_ip_number_to_large)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://256")
    , "PARSE ERROR [15]: expected 'number in [0,256)'\n"
      "protocoll://256 \n"
      "               ^\n"
    );
}

BOOST_AUTO_TEST_CASE (host_ip_missing_first_dot)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://255")
    , "PARSE ERROR [15]: expected '.'\n"
      "protocoll://255 \n"
      "               ^\n"
    );

  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://255:255")
    , "PARSE ERROR [15]: expected '.'\n"
      "protocoll://255 :255\n"
      "               ^\n"
    );
}

BOOST_AUTO_TEST_CASE (host_ip_missing_second_dot)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://255.255")
    , "PARSE ERROR [19]: expected '.'\n"
      "protocoll://255.255 \n"
      "                   ^\n"
    );
}

BOOST_AUTO_TEST_CASE (host_ip_missing_third_dot)
{
  fhg::util::boost::test::require_exception<fhg::util::parse::error::expected>
    ( boost::bind (&ctor, "protocoll://255.255.255")
    , "PARSE ERROR [23]: expected '.'\n"
      "protocoll://255.255.255 \n"
      "                       ^\n"
    );
}

BOOST_AUTO_TEST_CASE (host_star)
{
  fhg::util::url_t const url ("protocoll://*");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_star_port_start)
{
  fhg::util::url_t const url ("protocoll://*:*");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "*:*");
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (host_star_port_start_parameter)
{
  fhg::util::url_t const url ("protocoll://*:*?k=v");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "*:*");
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (no_host_but_parameter)
{
  fhg::util::url_t const url ("protocoll://?k=v");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "v");
}

BOOST_AUTO_TEST_CASE (parse_file_url)
{
  {
    fhg::util::url_t url ("file:///foo/bar/baz");
    BOOST_REQUIRE_EQUAL (url.type (), "file");
    BOOST_REQUIRE_EQUAL (url.path (), "/foo/bar/baz");
  }

  {
    fhg::util::url_t url ("file://foo/bar/baz");
    BOOST_REQUIRE_EQUAL (url.type (), "file");
    BOOST_REQUIRE_EQUAL (url.path (), "foo/bar/baz");
  }
}

BOOST_AUTO_TEST_CASE (url_io)
{
  const std::string url_string ("file:///foo/bar/baz?a=b&c=d&d=e");

  {
    std::stringstream sstr;

    fhg::util::url_t url_o (url_string);
    sstr << url_o;

    BOOST_REQUIRE_EQUAL (sstr.str (), url_string);

    fhg::util::url_t url_i (sstr.str());

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
