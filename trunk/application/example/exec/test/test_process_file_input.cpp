
#include <process.hpp>
#include <iostream>
#include <cstdlib>

#include <require.hpp>

int
main (int argc, char ** argv)
{
  if (argc < 2)
    {
      std::cerr << "usage: " << argv[0] << " path_to_interleave" << std::endl;

      exit (EXIT_FAILURE);
    }

  const std::string prog (argv[1]);
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

  REQUIRE (res[0] == '1');
  REQUIRE (res[1] == 'a');
  REQUIRE (res[2] == '2');
  REQUIRE (res[3] == 'b');
  REQUIRE (res[4] == '3');
  REQUIRE (res[5] == 'c');
  REQUIRE (res[6] == '4');
  REQUIRE (res[7] == 'd');
  REQUIRE (res[8] == '5');
  REQUIRE (res[9] == 'e');

  std::cout << "SUCCESS" << std::endl;

  return EXIT_SUCCESS;
}
