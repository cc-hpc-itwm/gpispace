// dimitri.blatner@itwm.fraunhofer.de

#include <fhg/revision.hpp>

#include <drts/client.hpp>
#include <drts/drts.hpp>

#include <we/expr/parse/parser.hpp>
#include <we/type/activity.hpp>
#include <we/type/net.hpp>
#include <we/type/value.hpp>

#include <fhg/util/parse/position.hpp>
#include <fhg/util/parse/require.hpp>

#include <boost/program_options.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main (int argc, char** argv) try
{
  namespace po = boost::program_options;

  std::string input ("/dev/stdin");
  std::string output ("/dev/stdout");
  std::vector<std::string> token_to_put;
  std::string control ("_CONTROL");
  unsigned long step_number = 1;
  std::vector<std::string> breakafter_spec;

  po::options_description desc ("options");

  desc.add_options()
    ( "help,h", "This message")
    ( "version,V", "Print version information")
    ( "input,i"
    , po::value<std::string> (&input)->default_value (input)
    , "Input file name of activity, - for stdin (default)"
    )
    ( "output,o"
    , po::value<std::string> (&output)->default_value (output)
    , "Output file name of activity, - for stdout (default)"
    )
    ( "put"
    , po::value<std::vector<std::string>> (&token_to_put)
    , "Put token on port, syntax: PORT_NAME=<VALUE>"
    )
    ( "step"
    , po::value<std::string> (&control )->implicit_value("_STEP")
    , "Executes one step for given input activity, which means only one transition fires and the activity terminates. Ignores --break-after if defined."
    )
    ( "step-number"
    , po::value<unsigned long> (&step_number)->default_value (step_number)
    , "Specify amount of steps to execute. Requires --step to be set. Default is 1."
    )
    ( "break-after"
    , po::value<std::vector<std::string>> (&breakafter_spec)
    , "Executes given input activity and breaks after one of the given transition(s) have fired. This is the default for all uspecified command-line arguments. Ignored if --step is defined."
    )
    ;

  //! \todo how to connect the options to the options of the debugged activity
  desc.add (gspc::options::installation());
  desc.add (gspc::options::drts());
  desc.add (gspc::options::logging());
  //! \todo find out whether or not the debugged activity requires virtual memory
  //desc.add (gspc::options::virtual_memory());

  po::positional_options_description p;
  p.add ("break-after", -1);

  po::variables_map vm;
  po::store ( po::command_line_parser (argc, argv)
            . options (desc).positional (p).run()
            , vm
            );

  if (vm.count ("help"))
  {
    std::cout << argv[0]
              << ": net debugging with activity transformations"
              << std::endl;
    std::cout << desc << std::endl;

    return EXIT_SUCCESS;
  }

  if (vm.count ("version"))
  {
    std::cout << fhg::project_info ("Net Debugger");

    return EXIT_SUCCESS;
  }

  po::notify (vm);

  //! \todo get the topology description that fits with the debugged activity
  std::string const topology_description ("work:8");

  gspc::scoped_runtime_system const drts
    (vm, gspc::installation (vm), topology_description, std::cerr, boost::none);

  gspc::workflow workflow (input);

  // put values on ports, string syntax "port_name=value"
  for (std::string const& port_and_value : token_to_put)
  {
    fhg::util::parse::position_string pos (port_and_value);
    fhg::util::parse::require::skip_spaces (pos);

    const std::string port_name (fhg::util::parse::require::identifier (pos));

    fhg::util::parse::require::skip_spaces (pos);
    fhg::util::parse::require::require (pos, '=');

    pnet::type::value::value_type const value
      (expr::parse::parser (fhg::util::parse::require::rest (pos)).eval_all());

    workflow.add_input (port_name, value);
  }

  if (vm.count ("step"))
  {
    gspc::client (drts).step (workflow, step_number);
  }
  else if (!breakafter_spec.empty())
  {
    gspc::client (drts).break_after (workflow, breakafter_spec);
  }
  else
  {
    std::cout << "No suitable parameter for debugging found, aborting..."
              << std::endl;

    return EXIT_FAILURE;
  }

  std::ofstream stream (output.c_str());

  if (!stream)
  {
    throw std::runtime_error
      ("could not open file " + output + " for writing");
  }

  stream << workflow.to_string();

  return EXIT_SUCCESS;
}
catch (const std::exception& e)
{
  std::cerr << e.what() << std::endl;
  return EXIT_FAILURE;
}
