
#include <rif/entry_point.hpp>

#include <fhg/revision.hpp>

#include <fhg/util/boost/program_options/validators/existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_file.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/executable_path.hpp>
#include <util-generic/join.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/program_options/separated_argument_list_parser.hpp>
#include <util-generic/read_lines.hpp>

#include <rif/strategy/meta.hpp>

#include <boost/program_options.hpp>

#include <iostream>
#include <vector>

namespace
{
  namespace option
  {
    constexpr const char* const hostfile {"hostfile"};
    constexpr const char* const port {"port"};
    constexpr const char* const strategy {"strategy"};
    constexpr char const* const strategy_parameters {"strategy-parameters"};
    constexpr char const* const strategy_parameters_description
      { "strategy specific parameters. for convenience specify"
        " via '<options...> RIF <strategy parameters...> FIR <options...>"
      };
    using strategy_parameters_type = std::vector<std::string>;
  }
}

int main (int argc, char** argv)
try
{
  std::vector<std::string> const strategies
    {fhg::rif::strategy::available_strategies()};

  boost::program_options::options_description options_description;
  options_description.add_options()
    ("help", "this message")
    ( option::port
    , boost::program_options::value
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
    , "port to listen on"
    )
    ( option::hostfile
    , boost::program_options::value
        <fhg::util::boost::program_options::nonempty_file>()->required()
    , "hostfile"
    )
    ( option::strategy
    , boost::program_options::value<std::string>()->required()
    , ("strategy: one of " + fhg::util::join (strategies, ", ").string()).c_str()
    )
    ( option::strategy_parameters
    , boost::program_options::value<option::strategy_parameters_type>()
      ->default_value (option::strategy_parameters_type(), "")->required()
    , option::strategy_parameters_description
    )
    ;

  boost::program_options::variables_map vm;
  boost::program_options::store
    ( boost::program_options::command_line_parser (argc, argv)
      .options (options_description)
    . extra_style_parser
        ( fhg::util::program_options::separated_argument_list_parser
            ("RIF", "FIR", option::strategy_parameters)
        )
      .run()
    , vm
    );

  if (vm.count ("help"))
  {
    std::cerr << fhg::project_info ( std::string (argv[0])
                                   + ": bootstrap the gspc rif deamon"
                                   ) << "\n";
    std::cerr << options_description << "\n";
    return 0;
  }

  boost::program_options::notify (vm);

  std::string const strategy (vm.at (option::strategy).as<std::string>());

  if (std::find (strategies.begin(), strategies.end(), strategy) == strategies.end())
  {
    throw std::invalid_argument
      (( boost::format ("invalid argument '%1%' for --%2%: one of %3%")
       % strategy
       % option::strategy
       % fhg::util::join (strategies, ", ")
       ).str()
      );
  }

  auto const result
    (fhg::rif::strategy::bootstrap
          ( strategy
          , fhg::util::read_lines
              ( vm.at (option::hostfile)
              . as<fhg::util::boost::program_options::nonempty_file>()
              )
          , vm.count (option::port)
          ? boost::make_optional<unsigned short>
            ( static_cast<unsigned long>
              ( vm.at (option::port)
              . as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
              )
            )
          : boost::none
          , boost::filesystem::canonical
              (fhg::util::executable_path().parent_path() / INSTALLATION_HOME)
          , vm.at (option::strategy_parameters)
          . as<option::strategy_parameters_type>()
          , std::cout
          )
    );

  std::unordered_map<std::string, std::string> const& real_hostnames
    (std::get<2> (result));

  for (auto const& entry_point : std::get<0> (result))
  {
    std::cout << entry_point.first << ' ' << entry_point.second
              << " (" << real_hostnames.at (entry_point.first) << ')'
              << '\n';
  }

  for (auto const& failure : std::get<1> (result))
  {
    std::cerr << failure.first << ": "
              << fhg::util::exception_printer (failure.second) << "\n";
  }

  return std::get<1> (result).empty() ? 0 : 1;
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';

  return 1;
}
