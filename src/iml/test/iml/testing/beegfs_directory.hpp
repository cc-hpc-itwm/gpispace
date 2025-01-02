// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace iml_test
{
  namespace options
  {
    ::boost::program_options::options_description beegfs_directory();
  }

  ::boost::filesystem::path beegfs_directory
    (::boost::program_options::variables_map const&);
}
