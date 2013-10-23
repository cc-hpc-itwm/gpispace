#define BOOST_TEST_MODULE GspcNetUtil
#include <boost/test/unit_test.hpp>

#include <gspc/net/util.hpp>

BOOST_AUTO_TEST_CASE (test_too_many_colons)
{
  const std::string input ("::1:0");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, 0u);
}

BOOST_AUTO_TEST_CASE (test_valid_ipv6_host)
{
  const std::string input ("[::1]");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, std::string::npos);
  BOOST_REQUIRE_EQUAL (host, "::1");
  BOOST_REQUIRE_EQUAL (port, "");
}

BOOST_AUTO_TEST_CASE (test_valid_ipv6_host_port)
{
  const std::string input ("[::1]:42");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, std::string::npos);
  BOOST_REQUIRE_EQUAL (host, "::1");
  BOOST_REQUIRE_EQUAL (port, "42");
}

BOOST_AUTO_TEST_CASE (test_valid_hostname)
{
  const std::string input ("host.a-b-c.de");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, std::string::npos);
  BOOST_REQUIRE_EQUAL (host, "host.a-b-c.de");
  BOOST_REQUIRE_EQUAL (port, "");
}

BOOST_AUTO_TEST_CASE (test_missing_opening_bracket)
{
  const std::string input ("host]:42");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, 4);
}

BOOST_AUTO_TEST_CASE (test_too_many_opening_brackets)
{
  const std::string input ("[[host:42");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, 1);
}

BOOST_AUTO_TEST_CASE (test_too_many_closing_brackets)
{
  const std::string input ("[host]]:42");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, 6);
}

BOOST_AUTO_TEST_CASE (test_missing_closing_bracket)
{
  const std::string input ("[host:42");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, input.size () - 1);
}

BOOST_AUTO_TEST_CASE (test_missing_port)
{
  const std::string input ("host:");

  std::string host, port;
  size_t n = gspc::net::split_host_port (input, host, port);

  BOOST_REQUIRE_EQUAL (n, input.size () - 1);
}

BOOST_AUTO_TEST_CASE (test_join_host_port)
{
  const std::string host ("host");
  const std::string port ("42");

  std::string host_port;

  gspc::net::join_host_port (host, port, host_port);

  BOOST_REQUIRE_EQUAL (host_port, "host:42");
}

BOOST_AUTO_TEST_CASE (test_join_host_with_colon)
{
  const std::string host ("::1");
  const std::string port ("42");

  std::string host_port;

  gspc::net::join_host_port (host, port, host_port);

  BOOST_REQUIRE_EQUAL (host_port, "[::1]:42");
}

BOOST_AUTO_TEST_CASE (test_join_host_with_opening_bracket)
{
  const std::string host ("[host");
  const std::string port ("42");

  std::string host_port;

  BOOST_CHECK_THROW ( gspc::net::join_host_port (host, port, host_port)
                    , std::invalid_argument
                    );
}

BOOST_AUTO_TEST_CASE (test_join_host_with_closing_bracket)
{
  const std::string host ("host]");
  const std::string port ("42");

  std::string host_port;

  BOOST_CHECK_THROW ( gspc::net::join_host_port (host, port, host_port)
                    , std::invalid_argument
                    );
}

BOOST_AUTO_TEST_CASE (test_join_host_with_space)
{
  const std::string host ("a b");
  const std::string port ("42");

  std::string host_port;

  BOOST_CHECK_THROW ( gspc::net::join_host_port (host, port, host_port)
                    , std::invalid_argument
                    );
}
