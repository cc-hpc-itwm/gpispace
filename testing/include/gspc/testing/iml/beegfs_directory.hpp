// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/program_options.hpp>

#include <filesystem>

namespace iml_test
{
  namespace options
  {
    ::boost::program_options::options_description beegfs_directory();
  }

  std::filesystem::path beegfs_directory
    (::boost::program_options::variables_map const&);
}
