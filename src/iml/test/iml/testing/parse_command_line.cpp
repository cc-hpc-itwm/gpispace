// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/testing/parse_command_line.hpp>

#include <boost/program_options/parsers.hpp>

namespace iml_test
{
  ::boost::program_options::variables_map parse_command_line
    ( int argc
    , char** argv
    , ::boost::program_options::options_description const& options
    )
  {
    ::boost::program_options::variables_map vm;
    ::boost::program_options::command_line_parser impl (argc, argv);
    impl.options (options);
    ::boost::program_options::store (impl.run(), vm);
    return vm;
  }
}
