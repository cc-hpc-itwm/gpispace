#define BOOST_TEST_MODULE URLTest
#include <boost/test/unit_test.hpp>

#include <fhg/util/url.hpp>
#include <fhg/util/url_io.hpp>

#include <sstream>
#include <iostream>

BOOST_AUTO_TEST_CASE (test_url_basics)
{
  using namespace fhg::util;

  url_t url;

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
  using namespace fhg::util;

  url_t url ("http://localhost:8080?foo=bar&bar=baz");

  BOOST_CHECK_EQUAL (url.type (), "http");
  BOOST_CHECK_EQUAL (url.path (), "localhost:8080");
  BOOST_CHECK_EQUAL (url.get ("foo"), "bar");
  BOOST_CHECK_EQUAL (url.get ("bar"), "baz");
}

BOOST_AUTO_TEST_CASE (test_invalid_type)
{
  using namespace fhg::util;

  try
  {
    url_t url ("://foo");
    BOOST_ERROR ("invalid (empty) type specified");
  }
  catch (std::invalid_argument const &)
  {
    // OK
  }
}

BOOST_AUTO_TEST_CASE (test_empty_param)
{
  using namespace fhg::util;

  try
  {
    url_t url ("file://bar?=baz");
    BOOST_ERROR ("invalid (empty) param specified");
  }
  catch (std::invalid_argument const &)
  {
    // OK
  }
}

BOOST_AUTO_TEST_CASE (parse_file_url)
{
  using namespace fhg::util;

  {
    url_t url ("file:///foo/bar/baz");
    BOOST_REQUIRE_EQUAL (url.type (), "file");
    BOOST_REQUIRE_EQUAL (url.path (), "/foo/bar/baz");
  }

  {
    url_t url ("file://foo/bar/baz");
    BOOST_REQUIRE_EQUAL (url.type (), "file");
    BOOST_REQUIRE_EQUAL (url.path (), "foo/bar/baz");
  }
}

BOOST_AUTO_TEST_CASE (url_io)
{
  using namespace fhg::util;

  const std::string url_string ("file:///foo/bar/baz?a=b&c=d&d=e");

  {
    std::stringstream sstr;

    url_t url_o (url_string);
    sstr << url_o;

    BOOST_REQUIRE_EQUAL (sstr.str (), url_string);

    url_t url_i;
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
