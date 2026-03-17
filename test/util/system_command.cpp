#include <cerrno>
#include <cstdlib>
#include <fstream>

int main (int argc, char** argv)
{
  if (argc != 3)
  {
    return EINVAL;
  }

  std::ofstream {argv[1]} << argv[2];

  return EXIT_SUCCESS;
}
