// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_file_output
#include <boost/test/unit_test.hpp>

#include <process.hpp>

#include <fhg/util/boost/test/flatten_nested_exceptions.hpp>
#include <fhg/util/boost/test/printer/vector.hpp>

BOOST_AUTO_TEST_CASE (process_file_output)
{
  BOOST_REQUIRE_GT (boost::unit_test::framework::master_test_suite().argc, 1);

  const std::string prog
    (boost::unit_test::framework::master_test_suite().argv[1]);
  const std::string command (prog + " %FILE1% %FILE2%");

  char buf[11] = "1a2b3c4d5e";

  char part1[5];
  char part2[5];

  process::file_buffer file1 (part1, 5, "%FILE1%");
  process::file_buffer file2 (part2, 5, "%FILE2%");

  process::file_buffer_list files_output;

  files_output.push_back (file1);
  files_output.push_back (file2);

  process::execute_return_type ret
    (process::execute ( command
                      , process::const_buffer (buf,10)
                      , process::buffer (nullptr,0)
                      , process::file_const_buffer_list ()
                      , files_output
                      )
    );

  BOOST_REQUIRE_EQUAL (ret.bytes_read_stdout, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_read_stderr, 0);
  {
    process::execute_return_type::size_list_type sizes;
    sizes.push_back (5);
    sizes.push_back (5);
    BOOST_REQUIRE_EQUAL (ret.bytes_read_files_output, sizes);
  }
  BOOST_REQUIRE_EQUAL (ret.exit_code, 0);

  BOOST_REQUIRE_EQUAL (part1[0], '1');
  BOOST_REQUIRE_EQUAL (part1[1], '2');
  BOOST_REQUIRE_EQUAL (part1[2], '3');
  BOOST_REQUIRE_EQUAL (part1[3], '4');
  BOOST_REQUIRE_EQUAL (part1[4], '5');

  BOOST_REQUIRE_EQUAL (part2[0], 'a');
  BOOST_REQUIRE_EQUAL (part2[1], 'b');
  BOOST_REQUIRE_EQUAL (part2[2], 'c');
  BOOST_REQUIRE_EQUAL (part2[3], 'd');
  BOOST_REQUIRE_EQUAL (part2[4], 'e');
}

BOOST_AUTO_TEST_CASE (unused_file_parameter_does_not_hang)
{
  //! \note Test case will hang on failure and hopefully time-out.

  char buffer[1024];

  process::execute_return_type ret
    (process::execute ( "/bin/true %FILE%"
                      , process::const_buffer (nullptr, 0)
                      , process::buffer (buffer, sizeof (buffer))
                      , process::file_const_buffer_list()
                      , {{buffer, sizeof (buffer), "%FILE%"}}
                      )
    );

  BOOST_REQUIRE_EQUAL (ret.exit_code, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_written_stdin, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_read_stdout, 0);
  BOOST_REQUIRE_EQUAL (ret.bytes_read_stderr, 0);
  //! \todo check that no file was read if information available
  const std::vector<std::size_t> expected_bytes_read_files_output {0};
  BOOST_REQUIRE_EQUAL
    (ret.bytes_read_files_output, expected_bytes_read_files_output);
}
