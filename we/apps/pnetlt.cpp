// mirko.rahn@itwm.fraunhofer.de

#include <we/type/transition.hpp>

//! \todo eliminate this include
#include <we/type/net.hpp>
#include <we/mgmt/type/activity.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>

#include <fhg/revision.hpp>

namespace
{
  void tl (std::ostream& os, const we::mgmt::type::activity_t& a)
  {
    os << a.transition().name() << std::endl;
  }
}

int
main (int argc, char** argv)
try
{
  namespace po = boost::program_options;

  std::string input;
  std::string output;

  po::options_description desc ("General");

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "input,i"
    , po::value<std::string>(&input)->default_value ("-")
    , "input file name, - for stdin, first positional parameter"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value ("-")
    , "output file name, - for stdout, second positional parameter"
    )
    ;

  po::positional_options_description p;
  p.add ("input", 1).add ("output", 2);

  po::variables_map vm;
  po::store ( po::command_line_parser (argc, argv)
            . options (desc).positional (p).run()
            , vm
            );
  po::notify(vm);

  if (vm.count ("help"))
    {
      std::cout << argv[0] << ": list transition names" << std::endl
                << desc << std::endl
        ;
      return EXIT_SUCCESS;
    }

  if (vm.count ("version"))
    {
      std::cout << fhg::project_info ("pnet2dot");

      return EXIT_SUCCESS;
    }

  const we::mgmt::type::activity_t act
    ( input == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (input))
    );

  if (output == "-")
  {
    tl (std::cout, act);
  }
  else
  {
    std::ofstream os (output.c_str());

    if (!os)
    {
      throw std::runtime_error ("failed to open " + output + " for writing");
    }

    tl (os, act);
  }

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
