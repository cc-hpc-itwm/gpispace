// mirko.rahn@itwm.fraunhofer.de

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include <we/we.hpp>

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  std::string input ("/dev/stdin");

  po::options_description desc ("options");

  desc.add_options ()
    ("help,h", "this message")
    ("input,i"
    , po::value<std::string> (&input)->default_value (input)
    , "input file, - for /dev/stdin"
    )
    ;

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

  // *********************************************************************** //

  std::ifstream in (input.c_str());

  we::activity_t act (we::util::text_codec::decode<we::activity_t> (in));

  std::cout << "got" << std::endl;
  std::cout << act << std::endl;

  return EXIT_SUCCESS;
}
