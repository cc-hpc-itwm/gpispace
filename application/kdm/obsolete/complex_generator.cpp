#include <sstream>
#include <fstream>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/program_options.hpp>

#include <we/we.hpp>
#include "complex_generator.hpp"

namespace po = boost::program_options;

int main (int argc, char ** argv)
{
  po::options_description desc("options");

  std::string cfg_file;

  desc.add_options()
    ("help,h", "this message")
    ("cfg", po::value<std::string>(&cfg_file), "config file")
    ;

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cerr << desc << std::endl;
      return EXIT_SUCCESS;
    }

  we::transition_t trans (kdm::kdm<we::activity_t>::generate());

  we::activity_t act ( trans );

  if (cfg_file.size())
  {
    act.add_input
      ( we::input_t::value_type
      ( we::token_t ( "config_file"
                    , literal::STRING()
                    , cfg_file
                    )
      , trans.input_port_by_name ("config_file")
      )
      );
  }

  std::cout << we::util::text_codec::encode (act);

  return EXIT_SUCCESS;
}
