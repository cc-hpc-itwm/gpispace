// Copyright (C) 2020-2021,2023-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <gspc/we/loader/exceptions.hpp>

#include <gspc/util/print_container.hpp>

#include <gspc/util/fmt/std/filesystem/path.formatter.hpp>
#include <gspc/util/join.formatter.hpp>
#include <algorithm>
#include <fmt/core.h>
#include <iterator>


  namespace gspc::we::loader
  {
    module_load_failed::module_load_failed
        ( std::filesystem::path const& file
        , std::string const& reason
        )
      : std::runtime_error
          { fmt::format ( "could not load module '{0}': {1}"
                        , file
                        , reason
                        )
          }
    {}

    module_not_found::module_not_found
        ( std::filesystem::path const& file
        , std::string const& search_path
        )
      : std::runtime_error
          { fmt::format ( "module '{0}' not found in '{1}'"
                        , file
                        , search_path
                        )
          }
    {}

    function_not_found::function_not_found
        ( std::filesystem::path const& module
        , std::string const& name
        )
      : std::runtime_error
          { fmt::format ( "function {0}::{1} not found"
                        , module
                        , name
                        )
          }
    {}

    duplicate_function::duplicate_function
        ( std::filesystem::path const& module
        , std::string const& name
        )
      : std::runtime_error
          { fmt::format ( "duplicate function {0}::{1}"
                        , module
                        , name
                        )
          }
    {}

    namespace
    {
      template<typename T>
        std::vector<T> left_overs (std::vector<T> lhs, std::vector<T> rhs)
      {
        std::sort (lhs.begin(), lhs.end());
        std::sort (rhs.begin(), rhs.end());
        std::vector<T> diff;
        std::set_difference
          ( lhs.begin(), lhs.end(), rhs.begin(), rhs.end()
          , std::inserter (diff, diff.begin())
          );
        return diff;
      }
    }

    module_does_not_unload::module_does_not_unload
        ( std::filesystem::path module
        , std::vector<std::filesystem::path> before
        , std::vector<std::filesystem::path> after
        )
      : module_does_not_unload (module, left_overs (after, before))
    {}
    module_does_not_unload::module_does_not_unload
        ( std::filesystem::path module
        , std::vector<std::filesystem::path> left_over
        )
      : std::runtime_error
          { fmt::format ( "module '{0}' does not properly unload on dlclose"
                          ", leaking {1} loaded in the process"
                        , module
                        , util::print_container ("{", ", ", "}", left_over)
                        )
          }
    {}

    function_does_not_unload::function_does_not_unload
        ( std::string module
        , std::string name
        , std::vector<std::filesystem::path> before
        , std::vector<std::filesystem::path> after
        )
      : function_does_not_unload
          (module, name, left_overs (after, before))
    {}
    function_does_not_unload::function_does_not_unload
        ( std::string module
        , std::string name
        , std::vector<std::filesystem::path> left_over
        )
      : std::runtime_error
          { fmt::format ( "function {0}::{1} dynamically opened libraries "
                            "but did not properly unload, leaking {2} "
                            "loaded in the process"
                        , module
                        , name
                        , util::print_container ("{", ", ", "}", left_over)
                        )
          }
    {}
  }
