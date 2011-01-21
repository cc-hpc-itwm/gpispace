
#include <process.hpp>
#include <iostream>
#include <cstdlib>

#include <require.hpp>

int
main (int argc, char ** argv)
{
  if (argc < 2)
    {
      std::cerr << "usage: " << argv[0] << " path_to_deinterleave" << std::endl;

      exit (EXIT_FAILURE);
    }

  const std::string prog (argv[1]);
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

  REQUIRE (part1[0] == '1');
  REQUIRE (part1[1] == '2');
  REQUIRE (part1[2] == '3');
  REQUIRE (part1[3] == '4');
  REQUIRE (part1[4] == '5');

  REQUIRE (part2[0] == 'a');
  REQUIRE (part2[1] == 'b');
  REQUIRE (part2[2] == 'c');
  REQUIRE (part2[3] == 'd');
  REQUIRE (part2[4] == 'e');

  std::cout << "SUCCESS" << std::endl;

  return EXIT_SUCCESS;
}
