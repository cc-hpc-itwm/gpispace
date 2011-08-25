// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/parser.hpp>

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <we/we.hpp>

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  std::string input ("/dev/stdin");
  std::string output ("/dev/stdout");
  bool xml (false);

  po::options_description desc("options");

  desc.add_options()
    ("help,h", "this message")
    ( "input,i"
    , po::value<std::string>(&input)->default_value(input)
    , "input file name, - for stdin"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value(output)
    , "output file name, - for stdout"
    )
    ( "xml,x"
    , po::value<bool>(&xml)->default_value(xml)
    , "write xml instead of text format"
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

  if (input == "-")
    {
      input = "/dev/stdin";
    }

  if (output == "-")
    {
      output = "/dev/stdout";
    }

  state->set_input (input);

  xml::parse::type::function_type f (xml::parse::frontend (*state, input));

  we::transition_t trans (f.synthesize<we::activity_t> (*state));

  we::type::optimize::optimize (trans, state->options_optimize());

  // WORK HERE: The xml dump from the transition
  if (0)
  {
    std::ofstream stream ("/dev/stderr");
    fhg::util::xml::xmlstream s (stream);

    we::type::dump::dump (s, trans);
  }

  delete state;

  const we::activity_t act (trans);


  std::ofstream out (output.c_str());

  out << ( xml
         ? we::util::xml_codec::encode (act)
         : we::util::text_codec::encode (act)
         )
    ;

  return EXIT_SUCCESS;
}
