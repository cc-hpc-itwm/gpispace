// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <testing/shared_directory.hpp>

#include <util-generic/boost/program_options/validators/existing_directory.hpp>

namespace test
{
  namespace validators = fhg::util::boost::program_options;

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

  ::boost::filesystem::path shared_directory
    (::boost::program_options::variables_map const& vm)
  {
    return vm.at (options::name::shared_directory)
      .as<validators::existing_directory>();
  }
}
