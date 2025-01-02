// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace test
{
  namespace options
  {
    ::boost::program_options::options_description fmt_directory();
  }

  ::boost::filesystem::path fmt_directory
    (::boost::program_options::variables_map const&);
}
