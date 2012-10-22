// mirko.rahn@itwm.fraunhofer.de

#include <xml/parse/parser.hpp>

#include <iostream>

#include <boost/program_options.hpp>
#include <boost/filesystem.hpp>

#include <we/we.hpp>

#include <fhg/revision.hpp>

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  std::string input ("/dev/stdin");
  std::string output ("/dev/stdout");
  bool xml (false);

  po::options_description desc("General");

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "input,i"
    , po::value<std::string>(&input)->default_value(input)
    , "input file name, - for stdin, first positional parameter"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value(output)
    , "output file name, - for stdout, second positional parameter"
    )
    ( "xml,x"
    , po::value<bool>(&xml)->default_value(xml)->implicit_value(true)
    , "write xml instead of text format"
    )
    ;

  xml::parse::state::type state;

  state.add_options (desc);

  po::positional_options_description p;
  p.add("input", 1).add("output",2);

  po::variables_map vm;

  try
  {
    po::store( po::command_line_parser(argc, argv)
             . options(desc).positional(p)
             . extra_parser (xml::parse::state::detail::reg_M)
             . run()
             , vm
             );
    po::notify(vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    return EXIT_FAILURE;
  }

  if (vm.count("help"))
    {
      std::cout << argv[0] << ": the petri net compiler" << std::endl;

      std::cout << desc << std::endl;

      return EXIT_SUCCESS;
    }

  if (vm.count("version"))
    {
      std::cout << fhg::project_info();

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

  try
  {
    xml::parse::type::function_type f (xml::parse::frontend (state, input));

    we::transition_t trans (f.synthesize (state));

    we::type::optimize::optimize (trans, state.options_optimize());

    // WORK HERE: The xml dump from the transition
#if 0
    {
      std::ofstream stream ("/dev/stderr");
      fhg::util::xml::xmlstream s (stream);

      we::type::dump::dump (s, trans);
    }
#endif

    const we::activity_t act (trans);


    std::ofstream out (output.c_str());

    out << ( xml
           ? we::util::xml_codec::encode (act)
           : we::util::text_codec::encode (act)
           )
      ;
  }
  catch (std::exception const & ex)
  {
    std::cerr << "pnetc: failed: " << ex.what() << std::endl;

    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
