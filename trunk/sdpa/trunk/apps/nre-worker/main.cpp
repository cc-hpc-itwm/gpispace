#include <fhglog/fhglog.hpp>
#include <sdpa/modules/ModuleLoader.hpp>
#include <fvm-pc/pc.hpp>

int main(int ac, char **av)
{
  fhg::log::Configurator::configure();

  using namespace sdpa::modules;
  // create the module loader
  ModuleLoader::ptr_t loader(ModuleLoader::create());

  for (int i = 1; i < ac; ++i)
  {
    loader->load(av[i]);
  }
}
