#pragma once

#include <boost/filesystem.hpp>
#include <boost/format.hpp>

#include <stdexcept>

namespace we
{
  namespace loader
  {
#define MEMBER(_name, _type...)                           \
    public:                                               \
      const _type& _name() const { return _ ## _name; }   \
    private:                                              \
      _type _ ## _name

    class module_load_failed : public std::runtime_error
    {
    public:
      explicit module_load_failed ( const std::string& file
                                  , const std::string& reason
                                  )
        : std::runtime_error
          ( ( boost::format ("could not load module '%1%': %2%")
            % file
            % reason
            ).str()
          )
        , _file (file)
        , _reason (reason)
      {}

      MEMBER (file, std::string);
      MEMBER (reason, std::string);
    };

    class module_not_found : public std::runtime_error
    {
    public:
      explicit module_not_found ( const std::string& file
                                , const std::string& search_path
                                )
        : std::runtime_error
          ( ( boost::format ("module '%1%' not found in '%2%'")
            % file
            % search_path
            ).str()
          )
        , _file (file)
        , _search_path (search_path)
      {}

      MEMBER (file, std::string);
      MEMBER (search_path, std::string);
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
        , _module (module)
        , _name (name)
      {}

      MEMBER (module, boost::filesystem::path);
      MEMBER (name, std::string);
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
        , _module (module)
        , _name (name)
      {}

      MEMBER (module, boost::filesystem::path);
      MEMBER (name, std::string);
    };
#undef MEMBER
  }
}
