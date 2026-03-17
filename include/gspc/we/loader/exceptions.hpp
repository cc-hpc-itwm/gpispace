// Copyright (C) 2010,2013-2015,2020-2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <filesystem>

#include <stdexcept>
#include <string>
#include <vector>


  namespace gspc::we::loader
  {
    struct module_load_failed : public std::runtime_error
    {
      module_load_failed ( std::filesystem::path const& file
                         , std::string const& reason
                         );
    };

    struct module_not_found : public std::runtime_error
    {
      module_not_found ( std::filesystem::path const& file
                       , std::string const& search_path
                       );
    };

    struct function_not_found : public std::runtime_error
    {
      function_not_found ( std::filesystem::path const& module
                         , std::string const& name
                         );
    };

    struct duplicate_function : public std::runtime_error
    {
      duplicate_function ( std::filesystem::path const& module
                         , std::string const& name
                         );
    };

    struct module_does_not_unload : public std::runtime_error
    {
      module_does_not_unload ( std::filesystem::path module
                             , std::vector<std::filesystem::path> before
                             , std::vector<std::filesystem::path> after
                             );

      module_does_not_unload ( std::filesystem::path module
                             , std::vector<std::filesystem::path> left_over
                             );
    };

    struct function_does_not_unload : public std::runtime_error
    {
      function_does_not_unload ( std::string module
                               , std::string name
                               , std::vector<std::filesystem::path> before
                               , std::vector<std::filesystem::path> after
                               );

      function_does_not_unload ( std::string module
                               , std::string name
                               , std::vector<std::filesystem::path> left_over
                               );
    };
  }
