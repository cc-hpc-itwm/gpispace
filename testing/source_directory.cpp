// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <testing/source_directory.hpp>

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
        constexpr char const* const source_directory {"source-directory"};
      }
    }

    ::boost::program_options::options_description source_directory()
    {
      ::boost::program_options::options_description source_directory;

      source_directory.add_options()
        ( name::source_directory
        , ::boost::program_options::value<validators::existing_directory>()
        ->required()
        , "source directory"
        )
        ;

      return source_directory;
    }
  }

  ::boost::filesystem::path source_directory
    (::boost::program_options::variables_map const& vm)
  {
    return vm.at (options::name::source_directory)
      .as<validators::existing_directory>();
  }
}
