// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <boost/filesystem/path.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace we
{
  namespace loader
  {
    struct module_load_failed : public std::runtime_error
    {
      module_load_failed ( boost::filesystem::path const& file
                         , std::string const& reason
                         );
    };

    struct module_not_found : public std::runtime_error
    {
      module_not_found ( boost::filesystem::path const& file
                       , std::string const& search_path
                       );
    };

    struct function_not_found : public std::runtime_error
    {
      function_not_found ( boost::filesystem::path const& module
                         , std::string const& name
                         );
    };

    struct duplicate_function : public std::runtime_error
    {
      duplicate_function ( boost::filesystem::path const& module
                         , std::string const& name
                         );
    };

    struct module_does_not_unload : public std::runtime_error
    {
      module_does_not_unload ( boost::filesystem::path module
                             , std::vector<boost::filesystem::path> before
                             , std::vector<boost::filesystem::path> after
                             );

      module_does_not_unload ( boost::filesystem::path module
                             , std::vector<boost::filesystem::path> left_over
                             );
    };

    struct function_does_not_unload : public std::runtime_error
    {
      function_does_not_unload ( std::string module
                               , std::string name
                               , std::vector<boost::filesystem::path> before
                               , std::vector<boost::filesystem::path> after
                               );

      function_does_not_unload ( std::string module
                               , std::string name
                               , std::vector<boost::filesystem::path> left_over
                               );
    };
  }
}
