#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>

#include <we/we.hpp>
#include "simple_generator.hpp"

// specific
#include "kdm_simple.hpp"

// generic
#include "module.hpp"
#include "context.hpp"

namespace po = boost::program_options;

int main (int argc, char ** argv)
{
  po::options_description desc("options");

  std::string cfg_file;
  std::string mod_path;
  std::vector<std::string> mods_to_load;

  desc.add_options()
    ("help", "this message")
    ("cfg", po::value<std::string>(&cfg_file)->default_value("/scratch/KDM/KDM.conf"), "config file")
    ("mod-path", po::value<std::string>(&mod_path)->default_value("/scratch/KDM/"), "modules")
    ("load", po::value<std::vector<std::string> >(&mods_to_load), "modules to load a priori")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

  we::loader::loader loader;

  loader.append_search_path (mod_path);
  for ( std::vector<std::string>::const_iterator m (mods_to_load.begin())
      ; m != mods_to_load.end()
      ; ++m
      )
  {
    loader.load (*m, *m);
  }

  we::transition_t simple_trans (kdm::kdm<we::activity_t>::generate());

  we::activity_t act ( simple_trans );

  act.input().push_back
    ( we::input_t::value_type
      ( we::token_t ( "config_file"
                    , literal::STRING
                    , cfg_file
                    )
      , simple_trans.input_port_by_name ("config_file")
      )
    );

  // dump activity for test purposes
  {
    std::ofstream ofs ("kdm_simple.pnet");
    ofs << we::util::text_codec::encode (act);
  }

  struct exec_context ctxt (loader);
  act.execute (ctxt);

  we::mgmt::type::detail::printer<we::activity_t, std::ostream> printer (act, std::cout);
  printer << "output := "
          << act.output()
          << std::endl;

  if ( act.output().empty() )
  {
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
