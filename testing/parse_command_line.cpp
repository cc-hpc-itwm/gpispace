// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <testing/parse_command_line.hpp>

#include <util-generic/boost/program_options/separated_argument_list_parser.hpp>
#include <drts/private/option.hpp>

#include <boost/program_options/parsers.hpp>

namespace test
{
  ::boost::program_options::variables_map parse_command_line
    ( std::vector<std::string> args
    , ::boost::program_options::options_description const& options
    )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::command_line_parser impl (args);
    impl.options (options);

    if (options.find_nothrow (gspc::name_rif_strategy_parameters(), false))
    {
      impl.extra_style_parser
        ( fhg::util::boost::program_options::separated_argument_list_parser
            ("RIF", "FIR", gspc::name_rif_strategy_parameters())
        );
    }

    ::boost::program_options::store (impl.run(), vm);
    return vm;
  }

  ::boost::program_options::variables_map parse_command_line
    ( int argc
    , char** argv
    , ::boost::program_options::options_description const& options
    )
  {
    std::vector<std::string> args;
    for (; argc != 0; ++argv, --argc)
    {
      args.emplace_back (*argv);
    }
    return parse_command_line (args, options);
  }
}
