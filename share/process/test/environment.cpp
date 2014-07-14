// alexander.petry@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_environment
#include <boost/test/unit_test.hpp>

#include <process.hpp>

BOOST_AUTO_TEST_CASE (process_empty_environment)
{
  BOOST_REQUIRE_EQUAL (boost::unit_test::framework::master_test_suite().argc, 2);

  const std::string env_program
    (boost::unit_test::framework::master_test_suite().argv[1]);

  std::map<std::string, std::string> env;

  process::circular_buffer buf_stderr;
  char out_stdout [0];

  process::execute_return_type ret
    ( process::execute ( env_program
                       , process::const_buffer (0, 0)
                       , process::buffer (out_stdout, sizeof (out_stdout))
                       , buf_stderr
                       , process::file_const_buffer_list()
                       , process::file_buffer_list()
                       , env
                       )
    );

  BOOST_REQUIRE_EQUAL (ret.exit_code, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_written_stdin, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_read_stdout, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_read_stderr, 0);
}

BOOST_AUTO_TEST_CASE (process_single_variable_environment)
{
  BOOST_REQUIRE_EQUAL (boost::unit_test::framework::master_test_suite().argc, 2);

  const std::string env_program
    (boost::unit_test::framework::master_test_suite().argv[1]);

  std::map<std::string, std::string> env;
  env ["FOO"] = "BAR";

  process::circular_buffer buf_stderr;
  char out_stdout [8];

  process::execute_return_type ret
    ( process::execute ( env_program
                       , process::const_buffer (0, 0)
                       , process::buffer (out_stdout, sizeof (out_stdout))
                       , buf_stderr
                       , process::file_const_buffer_list()
                       , process::file_buffer_list()
                       , env
                       )
    );

  BOOST_REQUIRE_EQUAL (ret.exit_code, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_written_stdin, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_read_stdout, 8);
  BOOST_REQUIRE_EQUAL (out_stdout[0], 'F');
  BOOST_REQUIRE_EQUAL (out_stdout[1], 'O');
  BOOST_REQUIRE_EQUAL (out_stdout[2], 'O');
  BOOST_REQUIRE_EQUAL (out_stdout[3], '=');
  BOOST_REQUIRE_EQUAL (out_stdout[4], 'B');
  BOOST_REQUIRE_EQUAL (out_stdout[5], 'A');
  BOOST_REQUIRE_EQUAL (out_stdout[6], 'R');
  BOOST_REQUIRE_EQUAL (out_stdout[7], '\n');

  BOOST_REQUIRE_EQUAL (ret.bytes_read_stderr, 0);
}

BOOST_AUTO_TEST_CASE (process_full_environment)
{
  BOOST_REQUIRE_EQUAL (boost::unit_test::framework::master_test_suite().argc, 2);

  const std::string env_program
    (boost::unit_test::framework::master_test_suite().argv[1]);

  std::string expected_output;
  {
    for (char **entry = environ; entry && *entry != nullptr; ++entry)
    {
      expected_output += *entry;
      expected_output += '\n';
    }
  }

  process::circular_buffer buf_stderr;
  std::vector<char> out_stdout (expected_output.size ());

  process::execute_return_type ret
    ( process::execute ( env_program
                       , process::const_buffer (0, 0)
                       , process::buffer (out_stdout.data (), out_stdout.size())
                       , buf_stderr
                       , process::file_const_buffer_list()
                       , process::file_buffer_list()
                       )
    );

  BOOST_REQUIRE_EQUAL (ret.exit_code, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_written_stdin, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_read_stdout, expected_output.size ());
  BOOST_REQUIRE_EQUAL (ret.bytes_read_stderr, 0);

  BOOST_REQUIRE_EQUAL (std::string (out_stdout.data (), ret.bytes_read_stdout), expected_output);
}
