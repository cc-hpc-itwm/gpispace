// mirko.rahn@itwm.fraunhofer.de

#define BOOST_TEST_MODULE process_file_output
#include <boost/test/unit_test.hpp>

#include <process.hpp>

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

  process::execute ( command
                   , process::const_buffer (buf,10)
                   , process::buffer (NULL,0)
                   , process::file_const_buffer_list ()
                   , files_output
                   );

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
