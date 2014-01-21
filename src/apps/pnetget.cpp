// mirko.rahn@itwm.fraunhofer.de
// alexander.petry@itwm.fraunhofer.de

#include <sysexits.h>

//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>
#include <we/type/activity.hpp>
#include <we/type/id.hpp>

#include <we/type/value/show.hpp>

#include <iostream>
#include <fstream>

#include <boost/program_options.hpp>
#include <boost/foreach.hpp>

#include <fhg/revision.hpp>

// ************************************************************************* //

namespace po = boost::program_options;

namespace detail
{
  template <typename In, typename Out, typename Pred>
  Out copy_if ( In first
              , In last
              , Out res
              , Pred pred
              )
  {
    while (first != last)
    {
      if (pred(*first))
        *res++ = *first;
      ++first;
    }
    return res;
  }
}

struct match_every_port
{
  bool operator() (const we::type::activity_t::token_on_port_t)
  {
    return true;
  }
};

struct match_equal_port
{
  match_equal_port(we::port_id_type p)
    : port(p)
  {}

  bool operator() (const we::type::activity_t::token_on_port_t & subject)
  {
    return subject.second == port;
  }
  const we::port_id_type port;
};

struct output_token
{
  output_token (std::ostream &os, std::string const & d="")
    : out (os)
    , delim(d)
  {}
  output_token const & operator *() const { return *this; }
  output_token const & operator++(int) const { return *this; }
  output_token const & operator=(const we::type::activity_t::token_on_port_t & subject) const
  {
    out << pnet::type::value::show (subject.first) << delim;
    return *this;
  }

  std::ostream & out;
  std::string delim;
};

struct output_port_and_token
{
  output_port_and_token (std::ostream &os, std::string const & d="")
    : out (os)
    , delim(d)
  {}
  output_port_and_token const & operator *() const { return *this; }
  output_port_and_token const & operator++(int) const { return *this; }
  output_port_and_token const & operator=(const we::type::activity_t::token_on_port_t & subject) const
  {
    out << "on " << subject.second << ": " << pnet::type::value::show (subject.first) << delim;
    return *this;
  }

  std::ostream & out;
  std::string delim;
};

int
main (int argc, char ** argv)
try
{
  std::string input ("-");
  std::string output ("-");
  std::vector<std::string> ports;
  std::string type("output");

  po::options_description desc("options");

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "port,p", po::value<std::vector<std::string> >(&ports), "port to retrieve tokens from" )
    ( "type,t", po::value<std::string>(&type)->default_value(type), "input/output port")
    ( "input,i"
    , po::value<std::string>(&input)->default_value(input)
    , "input file name, - for stdin"
    )
    ( "output,o"
    , po::value<std::string>(&output)->default_value(input)
    , "output file name, - for stdout"
    )
    ;

  po::positional_options_description p;
  p.add("port", -1);

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
      std::cout << argv[0] << ": get tokens from ports" << std::endl;

      std::cout << desc << std::endl;

      return EX_OK;
    }

  if (vm.count("version"))
  {
    std::cout << fhg::project_info ("Token Extractor");

    return EXIT_SUCCESS;
  }


  if (output == "-")
    {
      output = "/dev/stdout";
    }

  if (type != "input" && type != "output")
  {
    std::cerr << "invalid type specified: " << type << std::endl;
    return EX_USAGE;
  }

  we::type::activity_t act
    ( input == "-"
    ? we::type::activity_t (std::cin)
    : we::type::activity_t (boost::filesystem::path (input))
    );

  {
    std::ofstream stream (output.c_str());

    if (!stream)
      {
        std::cerr << "could not open file " + output + " for writing";
        return EX_CANTCREAT;
      }

    if (type == "input")
    {
      if (ports.size())
      {
        BOOST_FOREACH(std::string const &port, ports)
        {
          we::port_id_type port_id (0);
          try
          {
            port_id = act.transition().input_port_by_name(port);
          }
          catch (std::exception const &ex)
          {
            std::cerr << "no such input port: " << port << std::endl;
            return EX_USAGE;
          }
          detail::copy_if( act.input().begin()
                         , act.input().end()
                         , output_token(stream, "\n")
                         , match_equal_port(port_id)
                         );
        }
      }
      else
      {
        detail::copy_if( act.input().begin()
                       , act.input().end()
                       , output_port_and_token(stream, "\n")
                       , match_every_port()
                       );
      }
    }
    else
    {
      if (ports.size())
      {
        BOOST_FOREACH(std::string const &port, ports)
        {
          we::port_id_type port_id (0);
          try
          {
            port_id = act.transition().output_port_by_name(port);
          }
          catch (std::exception const &ex)
          {
            std::cerr << "no such output port: " << port << std::endl;
            return EX_USAGE;
          }
          we::type::activity_t::output_t output (act.output());

          detail::copy_if( output.begin()
                         , output.end()
                         , output_token(stream, "\n")
                         , match_equal_port(port_id)
                         );
        }
      }
      else
      {
        we::type::activity_t::output_t output (act.output());

        detail::copy_if( output.begin()
                       , output.end()
                       , output_port_and_token(stream, "\n")
                       , match_every_port()
                       );
      }
    }
  }

  return EX_OK;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
