// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <testing/parse_command_line.hpp>

#include <drts/private/option.hpp>
#include <util-generic/boost/program_options/separated_argument_list_parser.hpp>

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
