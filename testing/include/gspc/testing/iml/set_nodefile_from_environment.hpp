// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/program_options/variables_map.hpp>

#include <filesystem>

namespace iml_test
{
  std::filesystem::path set_nodefile_from_environment
    (::boost::program_options::variables_map&);
}
