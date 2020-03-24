#include <we/loader/exceptions.hpp>

#include <boost/format.hpp>

namespace we
{
  namespace loader
  {
    module_load_failed::module_load_failed
        ( boost::filesystem::path const& file
        , std::string const& reason
        )
      : std::runtime_error
          ( ( boost::format ("could not load module '%1%': %2%")
            % file
            % reason
            ).str()
          )
    {}

    module_not_found::module_not_found
        ( boost::filesystem::path const& file
        , std::string const& search_path
        )
      : std::runtime_error
          ( ( boost::format ("module '%1%' not found in '%2%'")
            % file
            % search_path
            ).str()
          )
    {}

    function_not_found::function_not_found
        ( boost::filesystem::path const& module
        , std::string const& name
        )
      : std::runtime_error
          ( ( boost::format ("function %1%::%2% not found")
            % module
            % name
            ).str()
          )
    {}

    duplicate_function::duplicate_function
        ( boost::filesystem::path const& module
        , std::string const& name
        )
      : std::runtime_error
          ( ( boost::format ("duplicate function %1%::%2%")
            % module
            % name
            ).str()
          )
    {}
  }
}
