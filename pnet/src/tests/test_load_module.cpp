#include <iostream>
#include <list>
#include <string>

#include <fhg/util/show.hpp>
#include <we/loader/loader.hpp>

int main (int ac, char **av)
{
  we::loader::loader loader;

  if (ac < 2)
  {
    std::cerr << "usage: " << av[0] << " module..." << std::endl;
    return EXIT_SUCCESS;
  }

  std::list<std::string> failed;
  for (int i = 1; i < ac; ++i)
  {
    const std::string mod_name ("mod-" + fhg::util::show(i));
    try
    {
      loader.load (mod_name, av[i]);
    }
    catch (const std::exception & ex)
    {
      std::cerr << "load of module " << av[i] << " failed: " << ex.what() << std::endl;
      failed.push_back (av[i]);
      continue;
    }

    try
    {
      expr::eval::context inp;
      expr::eval::context out;

      loader[mod_name] ("selftest", inp, out);
    }
    catch (const std::exception &ex)
    {
      std::cerr << "could not run module self-test: " << ex.what() << std::endl;
      failed.push_back (av[i]);
    }
  }

  std::cerr << loader << std::endl;

  if (failed.size())
  {
    std::cerr << "The following modules could not be loaded/tested:" << std::endl;
    for (std::list<std::string>::const_iterator m(failed.begin()); m != failed.end(); ++m)
    {
      std::cerr << "\t" << *m << std::endl;
    }
  }

  return (failed.empty() ? EXIT_SUCCESS : EXIT_FAILURE);
}
