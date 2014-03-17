#include <boost/format.hpp>

#include <iostream>
#include <stdexcept>
#include <string>

int main (int argc, char** argv)
{
  if (argc != 2)
  {
    throw std::runtime_error
      ((boost::format ("usage: %1% num_bytes") % argv[0]).str());
  }

  std::string const chars
    ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");
  std::string const compare
    ("ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

  int num_bytes (atoi (argv[1]));

  while (num_bytes > 0)
  {
    std::cout << chars << std::endl;
    std::cerr << compare << std::endl;

    num_bytes -= chars.size();
  }

  return 0;
}
