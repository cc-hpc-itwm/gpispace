#define BOOST_TEST_MODULE GspcRifCmdlineTests
#include <boost/test/unit_test.hpp>

#include <errno.h>
#include <gspc/rif/util.hpp>

struct F
{
  F ()
  {}
};

BOOST_FIXTURE_TEST_SUITE( suite, F )

BOOST_AUTO_TEST_CASE (test_simple_cmdline)
{
  std::vector<std::string> argv;
  size_t consumed;

  const std::string input ("echo hello world");
  gspc::rif::parse (input, argv, consumed);

  BOOST_REQUIRE_EQUAL (argv.size (), 3u);
  BOOST_REQUIRE_EQUAL (consumed, input.size ());
  BOOST_REQUIRE_EQUAL (argv [0], "echo");
  BOOST_REQUIRE_EQUAL (argv [1], "hello");
  BOOST_REQUIRE_EQUAL (argv [2], "world");
}

BOOST_AUTO_TEST_CASE (test_simple_cmdline_spaces)
{
  std::vector<std::string> argv;
  size_t consumed;

  gspc::rif::parse ("       echo    hello       world     ", argv, consumed);

  BOOST_REQUIRE_EQUAL (argv.size (), 3u);
  BOOST_REQUIRE_EQUAL (argv [0], "echo");
  BOOST_REQUIRE_EQUAL (argv [1], "hello");
  BOOST_REQUIRE_EQUAL (argv [2], "world");
}

BOOST_AUTO_TEST_CASE (test_single_quoted_arguments)
{
  std::vector<std::string> argv;
  size_t consumed;

  gspc::rif::parse ("echo \'hello world\' \'hello world\'", argv, consumed);

  BOOST_REQUIRE_EQUAL (argv.size (), 3u);
  BOOST_REQUIRE_EQUAL (argv [0], "echo");
  BOOST_REQUIRE_EQUAL (argv [1], "hello world");
  BOOST_REQUIRE_EQUAL (argv [2], "hello world");
}

BOOST_AUTO_TEST_CASE (test_double_quoted_arguments)
{
  std::vector<std::string> argv;
  size_t consumed;

  gspc::rif::parse ("echo \"hello world\"", argv, consumed);

  BOOST_REQUIRE_EQUAL (argv.size (), 2u);
  BOOST_REQUIRE_EQUAL (argv [0], "echo");
  BOOST_REQUIRE_EQUAL (argv [1], "hello world");
}

BOOST_AUTO_TEST_CASE (test_escape_sequences)
{
  {
    std::vector<std::string> argv;
    size_t consumed;
    gspc::rif::parse ("\\a \\b \\f \\n \\r \\t \\\\ \\\' \\\"", argv, consumed);

    BOOST_REQUIRE_EQUAL (argv.size (), 9u);
    BOOST_REQUIRE_EQUAL (argv [0], "\a");
    BOOST_REQUIRE_EQUAL (argv [1], "\b");
    BOOST_REQUIRE_EQUAL (argv [2], "\f");
    BOOST_REQUIRE_EQUAL (argv [3], "\n");
    BOOST_REQUIRE_EQUAL (argv [4], "\r");
    BOOST_REQUIRE_EQUAL (argv [5], "\t");
    BOOST_REQUIRE_EQUAL (argv [6], "\\");
    BOOST_REQUIRE_EQUAL (argv [7], "\'");
    BOOST_REQUIRE_EQUAL (argv [8], "\"");
  }

  {
    std::vector<std::string> argv;
    size_t consumed;
    gspc::rif::parse ("\\x00\\x01\\x02\\x03", argv, consumed);

    BOOST_REQUIRE_EQUAL (argv.size (), 1u);
    BOOST_REQUIRE_EQUAL (argv [0].size (), 4);
    BOOST_REQUIRE_EQUAL (argv [0][0], '\x00');
    BOOST_REQUIRE_EQUAL (argv [0][1], '\x01');
    BOOST_REQUIRE_EQUAL (argv [0][2], '\x02');
    BOOST_REQUIRE_EQUAL (argv [0][3], '\x03');
  }
}

BOOST_AUTO_TEST_CASE (test_invalid_hexcode)
{
  std::vector<std::string> argv;
  size_t consumed;
  int rc;
  rc = gspc::rif::parse ("\\xgh", argv, consumed);

  BOOST_REQUIRE_EQUAL (rc, -EINVAL);
  BOOST_REQUIRE_EQUAL (consumed, 2u);
  BOOST_REQUIRE_EQUAL (argv.size (), 0u);
}

BOOST_AUTO_TEST_SUITE_END()
