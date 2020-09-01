#pragma once

#include <util-generic/program_options/separated_argument_list_parser.hpp>
#include <iml/client/private/option.hpp>

#include <boost/program_options.hpp>

#include <string>

namespace iml_test
{
  boost::program_options::variables_map parse_command_line
    ( int argc
    , char** argv
    , boost::program_options::options_description const& options
    )
  {
    boost::program_options::variables_map vm;
    boost::program_options::command_line_parser impl (argc, argv);
    impl.options (options);

    if (options.find_nothrow (iml_client::name_rif_strategy_parameters(), false))
    {
      impl.extra_style_parser
        ( fhg::util::program_options::separated_argument_list_parser
            ("IML-RIF", "FIR-LMI", iml_client::name_rif_strategy_parameters())
        );
    }

    boost::program_options::store (impl.run(), vm);
    return vm;
  }
}
