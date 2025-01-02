// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/program_options/variables_map.hpp>

namespace iml_test
{
  ::boost::filesystem::path set_nodefile_from_environment
    (::boost::program_options::variables_map&);
}
