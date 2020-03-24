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
