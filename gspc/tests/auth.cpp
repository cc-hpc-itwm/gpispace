#define BOOST_TEST_MODULE GspcNetAuth
#include <boost/test/unit_test.hpp>

#include <gspc/net/auth/simple.hpp>
#include <gspc/net/auth/default_auth.hpp>

struct F
{
  F ()
    : m_cookie ("97bee4c6-efaf-4e28-8f4a-d2eedc06dd8d")
  {
    unsetenv ("GSPC_COOKIE");
  }

  std::string m_cookie;
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (test_simple_explicit)
{
  gspc::net::auth::simple_auth_t simple (m_cookie);

  BOOST_REQUIRE (    simple.is_authorized (m_cookie));
  BOOST_REQUIRE (not simple.is_authorized ("foo bar"));
  BOOST_REQUIRE (not simple.is_authorized (""));
}

BOOST_AUTO_TEST_CASE (test_simple_environment)
{
  setenv ("GSPC_COOKIE", m_cookie.c_str (), 1);

  gspc::net::auth::simple_auth_t simple;

  BOOST_REQUIRE (    simple.is_authorized (m_cookie));
  BOOST_REQUIRE (not simple.is_authorized ("foo bar"));
  BOOST_REQUIRE (not simple.is_authorized (""));
}

BOOST_AUTO_TEST_CASE (test_simple_default)
{
  gspc::net::auth::simple_auth_t simple;

  if (simple.get_cookie () == "")
  {
    // no environment variable set and user didn't have .gspc.cookie file
    BOOST_REQUIRE (simple.is_authorized (""));
    BOOST_REQUIRE (simple.is_authorized ("foo bar"));
    BOOST_REQUIRE (simple.is_authorized (m_cookie));
  }
  else
  {
    BOOST_REQUIRE (simple.is_authorized (simple.get_cookie ()));
    BOOST_REQUIRE (not simple.is_authorized (simple.get_cookie () + "foo"));
  }
}

BOOST_AUTO_TEST_CASE (test_default_auth)
{
  gspc::net::auth_t const & auth = gspc::net::auth::default_auth ();

  if (auth.get_cookie () == "")
  {
    // no environment variable set and user didn't have .gspc.cookie file
    BOOST_REQUIRE (auth.is_authorized (""));
    BOOST_REQUIRE (auth.is_authorized ("foo bar"));
    BOOST_REQUIRE (auth.is_authorized (m_cookie));
  }
  else
  {
    BOOST_REQUIRE (auth.is_authorized (auth.get_cookie ()));
    BOOST_REQUIRE (not auth.is_authorized (auth.get_cookie () + "foo"));
  }
}

BOOST_AUTO_TEST_SUITE_END()
