// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

namespace iml_test
{
  ::boost::program_options::variables_map parse_command_line
    ( int argc
    , char** argv
    , ::boost::program_options::options_description const& options
    );
}
