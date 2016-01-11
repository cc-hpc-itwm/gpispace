#include <iostream>
#include <stdexcept>

int main (int argc, char** argv)
try
{
  if (argc != 3)
  {
    throw std::invalid_argument ("usage: demo.bin n m");
  }

  std::cout << "RUN " << std::stoi (argv[1])
            << " of " << std::stoi (argv[2]) << '\n';

  return 0;
}
catch (std::exception const& e)
{
  std::cerr << "EXCEPTION: " << e.what() << '\n';

  return 1;
}
catch (...)
{
  std::cerr << "EXCEPTION: UNKNOWN\n";

  return 1;
}
