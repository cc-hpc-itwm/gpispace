//#include <sdpa/daemon/nre/nre-worker/nre-worker/nre-pcd.hpp>
#include <fhglog/fhglog.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <csignal>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include "ActivityExecutor.hpp"
#include <sdpa/daemon/nre/messages.hpp>

static bool verbose (false);

int main(int ac, char **av)
{
  typedef std::vector<std::string> mod_list;
  namespace po = boost::program_options;

  po::options_description opts("Available Options");
  // fill in defaults
  opts.add_options()
    ("help,h", "show this help text")
    ("location,l", po::value<std::string>()->default_value("127.0.0.1:8000"), "where to listen")
    ("load" , po::value<std::vector<std::string> >(), "shared modules that shall be loaded")
    ("append-search-path,a", po::value<std::vector<std::string> >(), "append path for the modules")
    ("prepend-search-path,p", po::value<std::vector<std::string> >(), "prepend path for the modules")
    ("verbose,v", "verbose output")
    ("keep-going,k", "keep going, even if the FVM is not there")
    ("rank,r", po::value<int>()->default_value(0), "the rank of this pc")
    ;

  po::positional_options_description positional; // positional parameters used for command line parsing
  positional.add("load", -1);

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(ac, av).options(opts).positional(positional).run(), vm);
  }
  catch (const std::exception &ex)
  {
    std::cerr << "E: could not parse command line: " << ex.what() << std::endl;
    std::cerr << opts << std::endl;
    return 2;
  }
  verbose = (vm.count("verbose") > 0);

  if (vm.count("help"))
  {
    std::cerr << "usage: nre-pcd [options] module...." << std::endl;
    std::cerr << opts << std::endl;
    return 1;
  }

  fhg::log::Configurator::configure();

  using namespace we::loader;

  LOG(INFO, "starting on location: " << vm["location"].as<std::string>() << "...");
  sdpa::shared_ptr<sdpa::nre::worker::ActivityExecutor> executor
    (new sdpa::nre::worker::ActivityExecutor( vm["location"].as<std::string>()
                                            , vm["rank"].as<int>()
                                            )
    );
  if (vm.count("prepend-search-path"))
  {
    const std::vector<std::string>& search_path= vm["prepend-search-path"].as<std::vector<std::string> >();

    for ( std::vector<std::string>::const_iterator p(search_path.begin())
        ; p != search_path.end()
        ; ++p
        )
    {
      executor->loader().prepend_search_path( *p );
    }
  }

  if (vm.count("append-search-path"))
  {
    const std::vector<std::string>& search_path= vm["append-search-path"].as<std::vector<std::string> >();

    for ( std::vector<std::string>::const_iterator p(search_path.begin())
        ; p != search_path.end()
        ; ++p
        )
    {
      executor->loader().append_search_path( *p );
    }
  }

  if (vm.count("load"))
  {
    try
    {
      const mod_list& cmdline_mods = vm["load"].as<std::vector<std::string> >();
      for ( mod_list::const_iterator mod(cmdline_mods.begin())
          ; mod != cmdline_mods.end()
          ; ++mod
          )
      {
        executor->loader().load(*mod, *mod);
      }
    }
    catch (const ModuleLoadFailed &mlf)
    {
      std::cerr << "could not load module: " << mlf.what() << std::endl;
      return 3;
    }
    catch (const std::exception &ex)
    {
      std::cerr << "could not load module: " << ex.what() << std::endl;
      return 3;
    }
  }

  return executor->run();

}
