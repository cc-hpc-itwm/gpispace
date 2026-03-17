// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/program_options.hpp>

#include <filesystem>

namespace gspc::testing
{
  namespace options
  {
    ::boost::program_options::options_description fmt_directory();
  }

  std::filesystem::path fmt_directory
    (::boost::program_options::variables_map const&);
}
