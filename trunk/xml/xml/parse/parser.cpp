// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/parser.hpp>

#include <iostream>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <we/we.hpp>

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  std::string input;
  std::string output;

  po::options_description desc("options");

  desc.add_options()
    ("help,h", "this message")
    ( "input,i"
    , po::value<std::string>(&input)->default_value("-")
    , "input file name, - for stdin"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value("-")
    , "output file name, - for stdout"
    )
    ;

  xml::parse::state::type * state = new xml::parse::state::type;

  state->add_options (desc);

  po::positional_options_description p;
  p.add("input", -1);

  po::variables_map vm;
  po::store( po::command_line_parser(argc, argv)
           . options(desc).positional(p).run()
           , vm
           );
  po::notify(vm);

  if (vm.count("help"))
    {
      std::cout << desc << std::endl;
      return EXIT_SUCCESS;
    }

  xml::parse::type::function_type f (xml::parse::frontend (*state, input));

  if (state->print_internal_structures())
    {
      std::cerr << f << std::endl;
    }

  // optimize f

  we::transition_t trans (f.synthesize<we::activity_t> (*state));

  // optimize trans

  we::type::optimize::optimize (trans, state->options_optimize());

  delete state;

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
