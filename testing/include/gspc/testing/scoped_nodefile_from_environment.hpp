// Copyright (C) 2014-2015,2021,2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <gspc/util/temporary_file.hpp>

#include <boost/program_options.hpp>

namespace gspc::testing
{
  class scoped_nodefile_from_environment
  {
  public:
    scoped_nodefile_from_environment
      ( std::filesystem::path const& shared_directory
      , ::boost::program_options::variables_map&
      );

    [[nodiscard]] auto path() const -> std::filesystem::path
    {
      return _temporary_file;
    }

  private:
    gspc::util::temporary_file const _temporary_file;
  };
}
