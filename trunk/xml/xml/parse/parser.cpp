// mirko.rahn@itwm.fraunhofer.de

#include <parse/parser.hpp>

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  std::string input;
  std::string output;

  po::options_description desc("options");

  desc.add_options()
    ("help", "this message")
    ( "input"
    , po::value<std::string>(&input)->default_value("-")
    , "input file name, - for stdin"
    )
    ( "output"
    , po::value<std::string>(&output)->default_value("-")
    , "output file name, - for stdout"
    )
    ;

  xml::parse::state::type state;
  state.add_options (desc);

  po::variables_map vm;
  po::store(po::parse_command_line(argc, argv, desc), vm);
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

  const we::transition_t trans (xml::parse::parse (state, input));
  const we::activity_t act (trans);

  if (output == "-")
    {
      std::cout << we::util::text_codec::encode (act);
    }
  else
    {
      std::ofstream out (output.c_str());
      
      out << we::util::text_codec::encode (act);
    }
  
  return EXIT_SUCCESS;
}
