// Copyright (C) 2014-2015,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>

#include <boost/program_options.hpp>

namespace gspc::testing
{
  namespace options
  {
    ::boost::program_options::options_description source_directory();
  }

  std::filesystem::path source_directory
    (::boost::program_options::variables_map const&);
}
