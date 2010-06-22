#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>

#include <we/we.hpp>
#include "simple_generator_parallel_loadTT.hpp"

// specific
//#include "kdm_simple.hpp"

// generic
//#include "module.hpp"
//#include "context.hpp"

namespace po = boost::program_options;

int main (int argc, char ** argv)
{
  po::options_description desc("options");

  std::string cfg_file;

  desc.add_options()
    ("help", "this message")
    ("cfg", po::value<std::string>(&cfg_file)->default_value("/p/herc/itwm/hpc/soft/sdpa/sdpa/share/kdm.pnet"), "config file")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cerr << desc << std::endl;
      return EXIT_SUCCESS;
    }

  we::transition_t simple_trans (kdm::kdm<we::activity_t>::generate());

  we::activity_t act ( simple_trans );

  act.add_input
    ( we::input_t::value_type
      ( we::token_t ( "config_file"
                    , literal::STRING
                    , cfg_file
                    )
      , simple_trans.input_port_by_name ("config_file")
      )
    );

  // dump activity for test purposes
  std::cout << we::util::text_codec::encode (act);

  return EXIT_SUCCESS;
}
