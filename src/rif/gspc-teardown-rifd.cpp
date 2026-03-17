// Copyright (C) 2015-2016,2018-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/rif/entry_point.hpp>

#include <gspc/configuration/info.hpp>

#include <gspc/util/boost/program_options/separated_argument_list_parser.hpp>
#include <gspc/util/boost/program_options/validators/existing_path.hpp>
#include <gspc/util/join.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/read_lines.hpp>

#include <gspc/rif/strategy/meta.hpp>

#include <boost/program_options.hpp>

#include <gspc/util/join.formatter.hpp>
#include <fmt/core.h>
#include <iostream>
#include <vector>

namespace
{
  namespace option
  {
    constexpr const char* const entry_points_file {"entry-points-file"};
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
    ( option::entry_points_file
    , ::boost::program_options::value
        <gspc::util::boost::program_options::existing_path>()->required()
    , "entry_points file"
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
                                            + ": tear down the gspc rif deamon"
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

  std::unordered_map<std::string, gspc::rif::entry_point> entry_points;

  for ( std::string line
      : gspc::util::read_lines
          ( static_cast<std::filesystem::path>
            ( vm.at (option::entry_points_file)
            . as<gspc::util::boost::program_options::existing_path>()
            )
          )
      )
  {
    std::string::size_type const pos (line.find_first_of (' '));

    if (pos == std::string::npos)
    {
      throw std::logic_error ("Failed to parse entry_points_file");
    }

    entry_points.emplace ( line.substr (0, pos)
                         , line.substr (pos + 1, std::string::npos)
                         );
  }

  auto const result
    ( gspc::rif::strategy::teardown
        ( strategy
        , entry_points
        , vm.at (option::strategy_parameters)
        . as<option::strategy_parameters_type>()
        )
    );

  for (auto const& failure : result.second)
  {
    std::cout << failure.first << ' ' << entry_points.at (failure.first) << '\n';
    std::cerr << failure.first << ": "
              << gspc::util::exception_printer (failure.second) << "\n";
  }

  return result.second.empty() ? 0 : 1;
}
catch (...)
{
  std::cerr << "EX: " << gspc::util::current_exception_printer() << '\n';

  return 1;
}
