
#include <process.hpp>
#include <iostream>
#include <cstdlib>

#include <require.hpp>

#include <fhglog/minimal.hpp>

int
main (int argc, char ** argv)
{
  FHGLOG_SETUP();

  if (argc < 2)
    {
      std::cerr << "usage: " << argv[0] << " path_to_rotate" << std::endl;

      exit (EXIT_FAILURE);
    }

  const std::string prog (argv[1]);
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

  REQUIRE (ret.bytes_read_stdout == 7);
  REQUIRE (out_stdout[0] == 'a');
  REQUIRE (out_stdout[1] == 'b');
  REQUIRE (out_stdout[2] == 'c');
  REQUIRE (out_stdout[3] == 'd');
  REQUIRE (out_stdout[4] == 'e');
  REQUIRE (out_stdout[5] == 'f');
  REQUIRE (out_stdout[6] == 'g');

  REQUIRE (ret.bytes_read_files_output.size() == 2);

  REQUIRE (ret.bytes_read_files_output[0] == 5);
  REQUIRE (out_file3[0] == 'H');
  REQUIRE (out_file3[1] == 'a');
  REQUIRE (out_file3[2] == 'l');
  REQUIRE (out_file3[3] == 'l');
  REQUIRE (out_file3[4] == 'o');

  REQUIRE (ret.bytes_read_files_output[1] == 3);
  REQUIRE (out_file4[0] == '1');
  REQUIRE (out_file4[1] == '2');
  REQUIRE (out_file4[2] == '3');

  REQUIRE (ret.bytes_read_stderr == 23);
  REQUIRE (  std::string (buf_stderr.begin(), buf_stderr.end())
          == "23 bytes sent to stderr"
          );

  std::cout << "SUCCESS" << std::endl;

  return EXIT_SUCCESS;
}
