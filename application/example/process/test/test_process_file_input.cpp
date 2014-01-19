// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_file_input
#include <boost/test/unit_test.hpp>

#include <process.hpp>

BOOST_AUTO_TEST_CASE (process_file_input)
{
  BOOST_REQUIRE_GT (boost::unit_test::framework::master_test_suite().argc, 1);

  const std::string prog
    (boost::unit_test::framework::master_test_suite().argv[1]);
  const std::string command (prog + " %FILE1% %FILE2%");

  char buf1[6] = "12345";
  char buf2[6] = "abcde";

  char res[10];

  process::file_const_buffer file1 (buf1, 5, "%FILE1%");
  process::file_const_buffer file2 (buf2, 5, "%FILE2%");

  process::file_const_buffer_list files_input;

  files_input.push_back (file1);
  files_input.push_back (file2);

  process::execute ( command
                   , process::const_buffer (NULL,0)
                   , process::buffer (res,10)
                   , files_input
                   , process::file_buffer_list ()
                   );

  BOOST_REQUIRE_EQUAL (res[0], '1');
  BOOST_REQUIRE_EQUAL (res[1], 'a');
  BOOST_REQUIRE_EQUAL (res[2], '2');
  BOOST_REQUIRE_EQUAL (res[3], 'b');
  BOOST_REQUIRE_EQUAL (res[4], '3');
  BOOST_REQUIRE_EQUAL (res[5], 'c');
  BOOST_REQUIRE_EQUAL (res[6], '4');
  BOOST_REQUIRE_EQUAL (res[7], 'd');
  BOOST_REQUIRE_EQUAL (res[8], '5');
  BOOST_REQUIRE_EQUAL (res[9], 'e');
}
