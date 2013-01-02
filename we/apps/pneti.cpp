// mirko.rahn@itwm.fraunhofer.de
// alexander.petry@itwm.fraunhofer.de

#include <sysexits.h>

#include <we/type/requirement.hpp>
//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>
#include <we/mgmt/type/activity.hpp>
#include <we/util/codec.hpp>

#include <fhg/revision.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

// ************************************************************************* //

typedef we::type::transition_t::requirements_t requirements_t;
typedef requirements_t::value_type requirement_t;

std::ostream & operator<<( std::ostream & os
                         , const requirement_t & req
                         )
{
  os << req.value() << " := " << (req.is_mandatory() ? "true" : "false")
    ;
  return os;
}

std::ostream & operator<<( std::ostream & os
                         , const requirements_t & requirements
                         )
{
  BOOST_FOREACH(requirement_t const & req, requirements)
  {
    os << req << std::endl;
  }
  return os;
}

// ************************************************************************* //

namespace po = boost::program_options;

int
main (int argc, char ** argv)
{
  std::string input ("-");

  po::options_description desc("options");

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "input,i"
    , po::value<std::string>(&input)->default_value(input)
    , "input file name, - for stdin"
    )
    ;

  po::positional_options_description p;
  p.add("input", -1);

  po::variables_map vm;
  try
  {
    po::store( po::command_line_parser(argc, argv)
             . options(desc).positional(p).run()
             , vm
             );
    po::notify (vm);
  }
  catch (std::exception const & ex)
  {
    std::cerr << "invalid argument: " << ex.what() << std::endl;
    std::cerr << "use " << argv[0] << " -h for help" << std::endl;
    return EX_USAGE;
  }

  if (vm.count("help"))
    {
      std::cout << argv[0] << ": list requirements" << std::endl;

      std::cout << desc << std::endl;

      return EX_OK;
    }

  if (vm.count("version"))
  {
    std::cout << fhg::project_info ("Requirement Listing");

    return 0;
  }

  if (input == "-")
    {
      input = "/dev/stdin";
    }

  we::mgmt::type::activity_t act;

  {
    std::ifstream stream (input.c_str());

    if (!stream)
      {
        std::cerr << "could not open file "
                  << input
                  << " for reading"
                  << std::endl
          ;
        return EX_NOINPUT;
      }

    try
    {
      we::util::codec::decode (stream, act);
    }
    catch (std::exception const & ex)
    {
      std::cerr << "could not parse input: " << ex.what() << std::endl;
      return EX_DATAERR;
    }
  }

  std::cout << act.transition().requirements();

  return EX_OK;
}
