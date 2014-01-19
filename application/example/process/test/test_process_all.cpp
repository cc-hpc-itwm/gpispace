// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_all
#include <boost/test/unit_test.hpp>

#include <process.hpp>

#include <fhglog/LogMacros.hpp>

#include <iostream>

BOOST_AUTO_TEST_CASE (process_all)
{
  FHGLOG_SETUP();

  BOOST_REQUIRE_GT (boost::unit_test::framework::master_test_suite().argc, 1);

  const std::string prog
    (boost::unit_test::framework::master_test_suite().argv[1]);
  const std::string command (prog + " %FILE1% %FILE2% %FILE3% %FILE4%");

  char in_stdin[6] = "Hallo";
  char in_file1[4] = "123";
  char in_file2[8] = "abcdefg";

  char out_stdout[7];
  char out_file3[5];
  char out_file4[3];

  process::file_const_buffer file1 (in_file1, 3, "%FILE1%");
  process::file_const_buffer file2 (in_file2, 7, "%FILE2%");

  process::file_const_buffer_list files_input;

  files_input.push_back (file1);
  files_input.push_back (file2);

  process::file_buffer file3 (out_file3, 5, "%FILE3%");
  process::file_buffer file4 (out_file4, 3, "%FILE4%");

  process::file_buffer_list files_output;

  files_output.push_back (file3);
  files_output.push_back (file4);

  process::circular_buffer buf_stderr;

  process::execute_return_type ret
    ( process::execute ( command
                       , process::const_buffer (in_stdin, 5)
                       , process::buffer (out_stdout, 7)
                       , buf_stderr
                       , files_input
                       , files_output
                       )
    );

  BOOST_REQUIRE_EQUAL (ret.bytes_read_stdout, 7);
  BOOST_REQUIRE_EQUAL (out_stdout[0], 'a');
  BOOST_REQUIRE_EQUAL (out_stdout[1], 'b');
  BOOST_REQUIRE_EQUAL (out_stdout[2], 'c');
  BOOST_REQUIRE_EQUAL (out_stdout[3], 'd');
  BOOST_REQUIRE_EQUAL (out_stdout[4], 'e');
  BOOST_REQUIRE_EQUAL (out_stdout[5], 'f');
  BOOST_REQUIRE_EQUAL (out_stdout[6], 'g');

  BOOST_REQUIRE_EQUAL (ret.bytes_read_files_output.size(), 2);

  BOOST_REQUIRE_EQUAL (ret.bytes_read_files_output[0], 5);
  BOOST_REQUIRE_EQUAL (out_file3[0], 'H');
  BOOST_REQUIRE_EQUAL (out_file3[1], 'a');
  BOOST_REQUIRE_EQUAL (out_file3[2], 'l');
  BOOST_REQUIRE_EQUAL (out_file3[3], 'l');
  BOOST_REQUIRE_EQUAL (out_file3[4], 'o');

  BOOST_REQUIRE_EQUAL (ret.bytes_read_files_output[1], 3);
  BOOST_REQUIRE_EQUAL (out_file4[0], '1');
  BOOST_REQUIRE_EQUAL (out_file4[1], '2');
  BOOST_REQUIRE_EQUAL (out_file4[2], '3');

  BOOST_REQUIRE_EQUAL (ret.bytes_read_stderr, 23);
  BOOST_REQUIRE_EQUAL
    ( std::string (buf_stderr.begin(), buf_stderr.end())
    , "23 bytes sent to stderr"
    );
}
