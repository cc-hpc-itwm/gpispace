// Copyright (C) 2023-2024,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/testing/fmt_directory.hpp>

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
        constexpr char const* const fmt_directory {"fmt-directory"};
      }
    }

    ::boost::program_options::options_description fmt_directory()
    {
      ::boost::program_options::options_description fmt_directory;

      fmt_directory.add_options()
        ( name::fmt_directory
        , ::boost::program_options::value<validators::existing_directory>()
        ->required()
        , "fmt directory"
        )
        ;

      return fmt_directory;
    }
  }

  std::filesystem::path fmt_directory
    (::boost::program_options::variables_map const& vm)
  {
    return vm.at (options::name::fmt_directory)
      . as<validators::existing_directory>()
      ;
  }
}
