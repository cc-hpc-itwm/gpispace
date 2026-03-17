// Copyright (C) 2015-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/configuration/info.hpp>

#include <gspc/util/boost/program_options/separated_argument_list_parser.hpp>
#include <gspc/util/boost/program_options/validators/existing_directory.hpp>
#include <gspc/util/boost/program_options/validators/nonempty_file.hpp>
#include <gspc/util/boost/program_options/validators/positive_integral.hpp>
#include <gspc/util/executable_path.hpp>
#include <gspc/util/join.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/read_lines.hpp>

#include <gspc/rif/strategy/meta.hpp>

#include <boost/program_options.hpp>

#include <filesystem>

#include <gspc/util/join.formatter.hpp>
#include <fmt/core.h>
#include <iostream>
#include <vector>
#include <optional>

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
    {gspc::rif::strategy::available_strategies()};

  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("help", "this message")
    ( option::port
    , ::boost::program_options::value
        <gspc::util::boost::program_options::positive_integral<unsigned short>>()
    , "port to listen on"
    )
    ( option::hostfile
    , ::boost::program_options::value
        <gspc::util::boost::program_options::nonempty_file>()->required()
    , "hostfile"
    )
    ( option::strategy
    , ::boost::program_options::value<std::string>()->required()
    , ("strategy: one of " + gspc::util::join (strategies, ", ").string()).c_str()
    )
    ( option::strategy_parameters
    , ::boost::program_options::value<option::strategy_parameters_type>()
      ->default_value (option::strategy_parameters_type(), "")->required()
    , option::strategy_parameters_description
    )
    ;

  ::boost::program_options::variables_map vm;
  ::boost::program_options::store
    ( ::boost::program_options::command_line_parser (argc, argv)
      .options (options_description)
    . extra_style_parser
        ( gspc::util::boost::program_options::separated_argument_list_parser
            ("RIF", "FIR", option::strategy_parameters)
        )
      .run()
    , vm
    );

  if (vm.count ("help"))
  {
    std::cerr << gspc::configuration::info ( std::string (argv[0])
                                            + ": bootstrap the gspc rif deamon"
                                            ) << "\n";
    std::cerr << options_description << "\n";
    return 0;
  }

  ::boost::program_options::notify (vm);

  std::string const strategy (vm.at (option::strategy).as<std::string>());

  if (std::find (strategies.begin(), strategies.end(), strategy) == strategies.end())
  {
    throw std::invalid_argument
      { fmt::format
        ( "invalid argument '{}' for --{}: one of {}"
        , strategy
        , option::strategy
        , gspc::util::join (strategies, ", ")
        )
      };
  }

  auto const result
    (gspc::rif::strategy::bootstrap
          ( strategy
          , gspc::util::read_lines
              ( static_cast<std::filesystem::path>
                ( vm.at (option::hostfile)
                . as<gspc::util::boost::program_options::nonempty_file>()
                )
              )
          , vm.count (option::port)
          ? std::make_optional<unsigned short>
            ( static_cast<unsigned short>
              ( vm.at (option::port)
              . as<gspc::util::boost::program_options::positive_integral<unsigned short>>()
              )
            )
          : std::nullopt
          , std::filesystem::canonical
              (gspc::util::executable_path().parent_path() / INSTALLATION_HOME)
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
              << gspc::util::exception_printer (failure.second) << "\n";
  }

  return std::get<1> (result).empty() ? 0 : 1;
}
catch (...)
{
  std::cerr << "EX: " << gspc::util::current_exception_printer() << '\n';

  return 1;
}
