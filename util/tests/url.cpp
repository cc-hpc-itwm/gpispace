#define BOOST_TEST_MODULE URLTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/url.hpp>
#include <fhg/util/url_io.hpp>

#include <fhg/util/boost/test/require_exception.hpp>

#include <boost/bind.hpp>
#include <boost/utility.hpp>

BOOST_AUTO_TEST_CASE (test_url_basics)
{
  fhg::util::url_t url;

  url
    .type ("http")
    .path ("localhost:8080")
    .set ("foo", "bar")
    .set ("bar", "baz")
    ;

  BOOST_CHECK_EQUAL (url.type (), "http");
  BOOST_CHECK_EQUAL (url.path (), "localhost:8080");
  BOOST_CHECK_EQUAL (url.get ("foo"), "bar");
  BOOST_CHECK_EQUAL (url.get ("bar"), "baz");
}

BOOST_AUTO_TEST_CASE (test_url_ctor)
{
  fhg::util::url_t url ("http://localhost:8080?foo=bar&bar=baz");

  BOOST_CHECK_EQUAL (url.type (), "http");
  BOOST_CHECK_EQUAL (url.path (), "localhost:8080");
  BOOST_CHECK_EQUAL (url.get ("foo"), "bar");
  BOOST_CHECK_EQUAL (url.get ("bar"), "baz");
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
  fhg::util::boost::test::require_exception<std::invalid_argument>
    (boost::bind (&ctor, "://foo"), "no type found in url: ://foo");
}

BOOST_AUTO_TEST_CASE (test_empty_param)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "file://bar?=baz")
    , "empty parameter in url: file://bar?=baz"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_path)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://&")
    , "malformed url: & not allowed in path"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_non_empty_path)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://path&")
    , "malformed url: & not allowed in path"
    );
}

BOOST_AUTO_TEST_CASE (ampersand_not_allowed_in_longer_path)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://longer/path&")
    , "malformed url: & not allowed in path"
    );
}

BOOST_AUTO_TEST_CASE (simple_type)
{
  fhg::util::url_t const url ("protocoll://");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (BROKEN_type_with_colon)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "pr:tocoll://")
    , "expected //"
    );
}

BOOST_AUTO_TEST_CASE (BROKEN_type_with_colon_slash)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "pr:/ocoll://")
    , "expected /"
    );
}

BOOST_AUTO_TEST_CASE (no_colon_required)
{
  fhg::util::url_t const url ("just_the_protocoll");

  BOOST_REQUIRE_EQUAL (url.type(), "just_the_protocoll");
  BOOST_REQUIRE (url.path().empty());
  BOOST_REQUIRE (url.args().empty());
}

BOOST_AUTO_TEST_CASE (empty_parameter_list)
{
  fhg::util::url_t const url ("protocoll://p/ath?");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "p/ath");
  BOOST_REQUIRE (url.args().empty());
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

BOOST_AUTO_TEST_CASE (BROKEN_key_contains_colon)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://p/ath?:=v")
    , "expected identifier"
    );
}

BOOST_AUTO_TEST_CASE (BROKEN_key_contains_colon_and_slashes)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://p/ath?://=v")
    , "expected identifier"
    );
}

BOOST_AUTO_TEST_CASE (BROKEN_value_contains_colon_and_slashes)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://p/ath?k=://")
    , "expected identifier"
    );
}

BOOST_AUTO_TEST_CASE (parameter_list_many)
{
  fhg::util::url_t const url ("protocoll://p/ath?1=a&2=b");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "p/ath");
  BOOST_REQUIRE_EQUAL (url.args().size(), 2);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "1");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "a");
  BOOST_REQUIRE_EQUAL (boost::next (url.args().begin())->first, "2");
  BOOST_REQUIRE_EQUAL (boost::next (url.args().begin())->second, "b");
}

BOOST_AUTO_TEST_CASE (BROKEN_key_contains_questionmark)
{
  fhg::util::url_t const url ("protocoll://p/ath?1=a&?=b");

  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://p/ath?1=a&?=b")
    , "expected identifier"
    );
}

BOOST_AUTO_TEST_CASE (BROKEN_value_contains_questionmark)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://p/ath?1=a&2=?")
    , "expected identifier"
    );
}

BOOST_AUTO_TEST_CASE (parameter_list_last_duplicate_wins)
{
  fhg::util::url_t const url ("protocoll://p/ath?k=1&k=2");

  BOOST_REQUIRE_EQUAL (url.type(), "protocoll");
  BOOST_REQUIRE_EQUAL (url.path(), "p/ath");
  BOOST_REQUIRE_EQUAL (url.args().size(), 1);
  BOOST_REQUIRE_EQUAL (url.args().begin()->first, "k");
  BOOST_REQUIRE_EQUAL (url.args().begin()->second, "2");
}


BOOST_AUTO_TEST_CASE (BROKEN_empty_value)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://path?key=")
    , "expected identifier"
    );
}

BOOST_AUTO_TEST_CASE (BROKEN_many_empty_value)
{
  fhg::util::boost::test::require_exception<std::invalid_argument>
    ( boost::bind (&ctor, "protocoll://path?1=&2=")
    , "expected identifier"
    );
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

    fhg::util::url_t url_i;
    sstr >> url_i;

    BOOST_CHECK_EQUAL (url_i.type ()  , url_o.type ());
    BOOST_CHECK_EQUAL (url_i.path ()  , url_o.path ());
    BOOST_CHECK_EQUAL (url_i.get ("a"), url_o.get ("a"));
    BOOST_CHECK_EQUAL (url_i.get ("c"), url_o.get ("c"));
    BOOST_CHECK_EQUAL (url_i.get ("d"), url_o.get ("d"));

    sstr.str ();

    sstr << url_i;
    BOOST_REQUIRE_EQUAL (sstr.str (), url_string);
  }
}
