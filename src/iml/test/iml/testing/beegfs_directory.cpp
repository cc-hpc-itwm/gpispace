// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <iml/testing/beegfs_directory.hpp>

#include <util-generic/boost/program_options/validators/existing_directory.hpp>

namespace iml_test
{
  namespace validators = fhg::util::boost::program_options;

  namespace options
  {
    namespace name
    {
      namespace
      {
        constexpr char const* const beegfs_directory {"beegfs-directory"};
      }
    }

    ::boost::program_options::options_description beegfs_directory()
    {
      ::boost::program_options::options_description beegfs_directory;

      beegfs_directory.add_options()
        ( name::beegfs_directory
        , ::boost::program_options::value<validators::existing_directory>()
        ->required()
        , "BeeGFS directory to store test data in"
        )
        ;

      return beegfs_directory;
    }
  }

  ::boost::filesystem::path beegfs_directory
    (::boost::program_options::variables_map const& vm)
  {
    return static_cast<::boost::filesystem::path>
      ( vm.at (options::name::beegfs_directory)
      . as<validators::existing_directory>()
      );
  }
}
