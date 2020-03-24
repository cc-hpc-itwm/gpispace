#pragma once

#include <boost/filesystem/path.hpp>

#include <stdexcept>
#include <string>

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
  }
}
