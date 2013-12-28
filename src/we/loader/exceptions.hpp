#ifndef WE_LOADER_EXCEPTIONS_HPP
#define WE_LOADER_EXCEPTIONS_HPP 1

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

      const std::string& file() const
      {
        return _file;
      }
      const std::string& reason() const
      {
        return _reason;
      }

    private:
      std::string _file;
      std::string _reason;
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

      const std::string& file() const
      {
        return _file;
      }
      const std::string& search_path() const
      {
        return _search_path;
      }

    private:
      std::string _file;
      std::string _search_path;
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

      const std::string& name() const
      {
        return _name;
      }

    private:
      std::string _name;
    };

    /**
       All exceptions related to a module.
    */
    class ModuleException : public std::runtime_error {
    public:
      ModuleException(const std::string &a_reason, const std::string &a_module)
        : std::runtime_error(a_reason), module_(a_module) {}

      virtual ~ModuleException() throw() {}
      const std::string &module() const { return module_; }
    private:
      std::string module_;
    };

    /**
       All exceptions related to a specific function.
    */
    class FunctionException : public ModuleException {
    public:
      FunctionException(const std::string &a_reason, const std::string &a_module, const std::string &a_function)
        : ModuleException(a_reason, a_module), function_(a_function) {}

      virtual ~FunctionException() throw() {}

      const std::string &function() const { return function_; }
    private:
      std::string function_;
    };

    class FunctionNotFound : public FunctionException {
    public:
      FunctionNotFound(const std::string &a_module, const std::string &a_function)
        : FunctionException("function could not be found: "+a_function, a_module, a_function) {}
      virtual ~FunctionNotFound() throw() {}
    };

    class DuplicateFunction : public FunctionException {
    public:
      DuplicateFunction(const std::string &a_module, const std::string &a_function)
        : FunctionException("duplicate function detected: " + a_module+"."+a_function, a_module, a_function) {}
      virtual ~DuplicateFunction() throw() {}
    };
  }
}

#endif
