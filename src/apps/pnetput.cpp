// mirko.rahn@itwm.fraunhofer.de

#include <fhg/revision.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/mgmt/type/activity.hpp>
//! \todo eliminate this include (that completes type transition_t::data)
#include <we/type/net.hpp>
#include <we/type/value.hpp>
#include <we/type/value/show.hpp>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>
#include <boost/unordered_map.hpp>

#include <fstream>
#include <iostream>

int main (int argc, char** argv) try
{
  namespace po = boost::program_options;

  std::string input ("-");
  std::string output ("-");
  std::vector<std::string> input_spec;

  po::options_description desc ("options");

  desc.add_options()
    ( "help,h", "this message")
    ( "version,V", "print version information")
    ( "if,i"
    , po::value<std::string> (&input)->default_value (input)
    , "input file name, - for stdin"
    )
    ( "of,o"
    , po::value<std::string> (&output)->default_value (input)
    , "output file name, - for stdout"
    )
    ( "put,p"
    , po::value<std::vector<std::string> > (&input_spec)
    , "input token: port=<value>"
    )
    ;

  po::positional_options_description p;
  p.add ("put", -1);

  po::variables_map vm;
  po::store ( po::command_line_parser (argc, argv)
            . options (desc).positional (p).run()
            , vm
            );
  po::notify (vm);

  if (vm.count ("help"))
  {
    std::cout << argv[0] << ": put tokens on input ports" << std::endl;
    std::cout << desc << std::endl;

    return EXIT_SUCCESS;
  }

  if (vm.count ("version"))
  {
    std::cout << fhg::project_info ("Token Injector");

    return EXIT_SUCCESS;
  }

  we::mgmt::type::activity_t act
    ( input == "-"
    ? we::mgmt::type::activity_t (std::cin)
    : we::mgmt::type::activity_t (boost::filesystem::path (input))
    );

  typedef boost::unordered_map< std::string
                              , pnet::type::value::value_type
                              > port_values_type;

  port_values_type port_values;

  BOOST_FOREACH (std::string const& inp, input_spec)
  {
    const std::string port_name (inp.substr (0, inp.find ('=')));
    const std::string value (inp.substr (inp.find ('=') + 1));

    const pnet::type::value::value_type val
      (expr::parse::parser (value).eval_all());

    if (not act.transition().net())
    {
      const port_values_type::const_iterator pos
        (port_values.find (port_name));

      if (pos != port_values.end())
      {
        std::cerr
          << "WARNING! On port " << port_name
          << " the put of value " << pnet::type::value::show (val)
          << " overwrites the put of value "
          << pnet::type::value::show (pos->second)
          << std::endl;
      }

      port_values[port_name] = val;
    }

    act.add_input (act.transition().input_port_by_name (port_name), val);
  }

  if (output == "-")
  {
    std::cout << act.to_string();
  }
  else
  {
    std::ofstream stream (output.c_str());

    if (!stream)
    {
      throw std::runtime_error
        ("could not open file " + output + " for writing");
    }

    stream << act.to_string();
  }

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
