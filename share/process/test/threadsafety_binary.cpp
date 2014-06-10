// bernd.loerwald@itwm.fraunhofer.de

#include <array>
#include <cassert>
#include <cstdio>
#include <iostream>
#include <string>

#include <fcntl.h>
#include <unistd.h>

int main (int argc, char** argv)
{
  assert (argc == 4);

  const std::size_t thread_id (std::strtoull (argv[1], nullptr, 10));

  const std::string thread_id_string (std::to_string (thread_id));

  const std::string expected_stdin (thread_id_string + " STDIN");
  const std::string stdout (thread_id_string + " STDOUT");
  const std::string stderr (thread_id_string + " STDERR");
  const std::string expected_file_in (thread_id_string + " FILE_IN");
  const std::string file_out (thread_id_string + " FILE_OUT");

  std::array<char, 2 << 20> buffer;

  const int file_in_fd (open (argv[2], O_RDONLY));
  const int file_out_fd (open (argv[3], O_WRONLY));

  const std::string stdin
    ( buffer.data()
    , buffer.data() + read (STDIN_FILENO, buffer.data(), buffer.size())
    );
  const std::string file_in
    ( buffer.data()
    , buffer.data() + read (file_in_fd, buffer.data(), buffer.size())
    );

  write (STDOUT_FILENO, stdout.c_str(), stdout.size());
  write (STDERR_FILENO, stderr.c_str(), stderr.size());

  write (file_out_fd, file_out.c_str(), file_out.size());

  close (file_in_fd);
  close (file_out_fd);

  int failflags = 0;

  if (stdin != expected_stdin)
  {
    std::cerr << "STDIN: '" << stdin << "' != '" << expected_stdin << "'\n";
    failflags |= 1;
  }
  if (file_in != expected_file_in)
  {
    std::cerr << "STDIN: '" << file_in << "' != '" << expected_file_in << "'\n";
    failflags |= 2;
  }

  return failflags;
}
