#include <iostream>
#include <fhg/plugin/core/plugin.hpp>

int main(int ac, char **av)
{
  if (ac <= 1)
  {
    std::cerr << "usage: " << av[0] << " path-to-plugin" << std::endl;
    return EXIT_FAILURE;
  }

  try
  {
    fhg::core::plugin_t::ptr_t p (fhg::core::plugin_t::create(av[1], true));
    std::cout << "name: " << p->name() << std::endl;
    std::cout << "desc: " << p->descriptor()->description << std::endl;
    std::cout << "author: " << p->descriptor()->author << std::endl;
    std::cout << "version: " << p->descriptor()->version << std::endl;
    std::cout << "license: " << p->descriptor()->license << std::endl;
    std::cout << "depends: " << p->descriptor()->depends << std::endl;
    std::cout << "key: " << p->descriptor()->featurekey << std::endl;
    std::cout << "magic: " << p->descriptor()->magic << std::endl;

    if (std::string(FHG_PLUGIN_VERSION_MAGIC) != p->descriptor()->magic)
    {
      std::cout << "my magic: " << FHG_PLUGIN_VERSION_MAGIC << std::endl;
      std::cout << "WARNING: version magics differ, this module might be incompatible!" << std::endl;
    }
  }
  catch (std::exception const &ex)
  {
    std::cerr << ex.what() << std::endl;
    return EXIT_FAILURE;
  }
}
