#include <iml/fuse/client.hpp>

#include <util-generic/print_exception.hpp>

#include <iostream>
#include <stdexcept>

int main (int argc, char** argv)
try
{
  // \todo Command line parsing.
  if (argc != 3)
  {
    throw std::invalid_argument ("usage: <exe> mountpoint iml-socket");
  }

  iml::fuse::client (argv[1], argv[2]);

  return 0;
}
catch (...)
{
  std::cerr << fhg::util::current_exception_printer() << "\n";
  return 1;
}
