
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
      std::cerr << "usage: " << argv[0] << " size" << std::endl;

      exit (EXIT_FAILURE);
    }

  const std::size_t size (atoi (argv[1]));
  const std::size_t count (size / sizeof (int));

  int * in = new int[count];
  int * out = new int[count];

  for (std::size_t i (0); i < count; ++i)
    {
      in[i] = i;
      out[i] = 0;
    }

  {
    const std::size_t ret (process::execute ("cat", in, size, out, size));

    REQUIRE (ret == size);

    for (std::size_t i (0); i < count; ++i)
      {
        REQUIRE (in[i] == out[i]);
      }
  }

  // inplace
  {
    const std::size_t ret (process::execute ("cat", in, size, in, size));

    REQUIRE (ret == size);

    for (std::size_t i (0); i < count; ++i)
      {
        REQUIRE (in[i] == out[i]);
      }
  }

  delete[] in;
  delete[] out;

  std::cout << "SUCCESS" << std::endl;

  return EXIT_SUCCESS;
}
