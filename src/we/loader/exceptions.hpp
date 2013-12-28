#ifndef WE_LOADER_EXCEPTIONS_HPP
#define WE_LOADER_EXCEPTIONS_HPP 1

#include <fhg/util/macros.hpp>

#include <boost/format.hpp>

#include <stdexcept>

namespace we
{
  namespace loader
  {
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
      virtual ~module_load_failed() throw() {}

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
      virtual ~module_not_found() throw() {}

      MEMBER (file, std::string);
      MEMBER (search_path, std::string);
    };

    class module_already_registered : public std::runtime_error
    {
    public:
      explicit module_already_registered (const std::string& name)
        : std::runtime_error
          ((boost::format ("module '%1%' already registered") % name).str())
        , _name (name)
      {}
      virtual ~module_already_registered() throw() {}

      MEMBER (name, std::string);
    };

    class function_not_found : public std::runtime_error
    {
    public:
      explicit function_not_found ( std::string const& module
                                  , std::string const& name
                                  )
        : std::runtime_error
          ( ( boost::format ("function '%1%::%2%' not found")
            % module
            % name
            ).str()
          )
        , _module (module)
        , _name (name)
      {}
      virtual ~function_not_found() throw() {}

      MEMBER (module, std::string);
      MEMBER (name, std::string);
    };

    class duplicate_function : public std::runtime_error
    {
    public:
      explicit duplicate_function ( std::string const& module
                                  , std::string const& name
                                  )
        : std::runtime_error
          ( ( boost::format ("duplicate function '%1%::%2%'")
            % module
            % name
            ).str()
          )
        , _module (module)
        , _name (name)
      {}
      virtual ~duplicate_function() throw() {}

      MEMBER (module, std::string);
      MEMBER (name, std::string);
    };
  }
}

#endif
