#pragma once

#include <boost/filesystem/path.hpp>
#include <boost/format.hpp>

#include <stdexcept>
#include <string>

namespace we
{
  namespace loader
  {
    class module_load_failed : public std::runtime_error
    {
    public:
      explicit module_load_failed ( boost::filesystem::path const& file
                                  , const std::string& reason
                                  )
        : std::runtime_error
          ( ( boost::format ("could not load module '%1%': %2%")
            % file
            % reason
            ).str()
          )
      {}
    };

    class module_not_found : public std::runtime_error
    {
    public:
      explicit module_not_found ( boost::filesystem::path const& file
                                , const std::string& search_path
                                )
        : std::runtime_error
          ( ( boost::format ("module '%1%' not found in '%2%'")
            % file
            % search_path
            ).str()
          )
      {}
    };

    class function_not_found : public std::runtime_error
    {
    public:
      explicit function_not_found ( boost::filesystem::path const& module
                                  , std::string const& name
                                  )
        : std::runtime_error
          ( ( boost::format ("function %1%::%2% not found")
            % module
            % name
            ).str()
          )
      {}
    };

    class duplicate_function : public std::runtime_error
    {
    public:
      explicit duplicate_function ( boost::filesystem::path const& module
                                  , std::string const& name
                                  )
        : std::runtime_error
          ( ( boost::format ("duplicate function %1%::%2%")
            % module
            % name
            ).str()
          )
      {}
    };
  }
}
