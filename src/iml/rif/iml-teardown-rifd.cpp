// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <iml/rif/EntryPoints.hpp>
#include <iml/rif/strategies.hpp>
#include <iml/rif/teardown.hpp>

#include <util-generic/boost/program_options/validators/existing_path.hpp>

#include <util-generic/boost/program_options/separated_argument_list_parser.hpp>
#include <util-generic/join.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_lines.hpp>

#include <boost/format.hpp>
#include <boost/program_options.hpp>

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
  auto const strategies (iml::rif::available_strategies());

  ::boost::program_options::options_description options_description;
  options_description.add_options()
    ("help", "this message")
    ( option::entry_points_file
    , ::boost::program_options::value
        <fhg::util::boost::program_options::existing_path>()->required()
    , "entry_points file"
    )
    ( option::strategy
    , ::boost::program_options::value<std::string>()->required()
    , ("strategy: one of " + fhg::util::join (strategies, ", ").string()).c_str()
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
        ( fhg::util::boost::program_options::separated_argument_list_parser
            ("RIF", "FIR", option::strategy_parameters)
        )
      .run()
    , vm
    );

  if (vm.count ("help"))
  {
    std::cerr << options_description << "\n";
    return 0;
  }

  ::boost::program_options::notify (vm);

  std::string const strategy (vm.at (option::strategy).as<std::string>());

  if (std::find (strategies.begin(), strategies.end(), strategy) == strategies.end())
  {
    throw std::invalid_argument
      (( ::boost::format ("invalid argument '%1%' for --%2%: one of %3%")
       % strategy
       % option::strategy
       % fhg::util::join (strategies, ", ")
       ).str()
      );
  }

  auto const entry_points
    ( iml::rif::read_from_file
        ( vm.at (option::entry_points_file)
        . as<fhg::util::boost::program_options::existing_path>()
        )
    );

  auto const result
    ( iml::rif::teardown
        ( entry_points
        , strategy
        , vm.at (option::strategy_parameters)
        . as<option::strategy_parameters_type>()
        )
    );

  for (auto const& failure : result.failures_by_host)
  {
    std::cout << failure.first << ' ' << entry_points.at (failure.first) << '\n';
    std::cerr << failure.first << ": "
              << fhg::util::exception_printer (failure.second) << "\n";
  }

  return result.failures_by_host.empty() ? 0 : 1;
}
catch (...)
{
  std::cerr << "EX: " << fhg::util::current_exception_printer() << '\n';

  return 1;
}
