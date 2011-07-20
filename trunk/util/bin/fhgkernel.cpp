#include <iostream>
#include <fhg/plugin/core/kernel.hpp>

int main(int ac, char **av)
{
  if (ac <= 1)
  {
    std::cerr << "usage: " << av[0] << " path-to-plugin" << std::endl;
    return EXIT_FAILURE;
  }

  fhg::core::kernel_t kernel;


  for (int i = 1 ; i < ac ; ++i)
  {
    try
    {
      kernel.load_plugin (av[i]);
    }
    catch (std::exception const &ex)
    {
      std::cerr << ex.what() << std::endl;
    }
  }
}
