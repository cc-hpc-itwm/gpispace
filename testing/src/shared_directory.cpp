// Copyright (C) 2014,2021,2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/shared_directory.hpp>

#include <gspc/util/boost/program_options/validators/existing_directory.hpp>

namespace gspc::testing
{
  namespace validators = gspc::util::boost::program_options;

  namespace options
  {
    namespace name
    {
      namespace
      {
        constexpr char const* const shared_directory {"shared-directory"};
      }
    }

    ::boost::program_options::options_description shared_directory()
    {
      ::boost::program_options::options_description shared_directory;

      shared_directory.add_options()
        ( name::shared_directory
        , ::boost::program_options::value<validators::existing_directory>()
        ->required()
        , "directory shared between workers"
        )
        ;

      return shared_directory;
    }
  }

  std::filesystem::path shared_directory
    (::boost::program_options::variables_map const& vm)
  {
    return vm.at (options::name::shared_directory)
      .as<validators::existing_directory>();
  }
}
