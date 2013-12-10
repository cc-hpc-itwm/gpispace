#ifndef WE_LOADER_EXCEPTIONS_HPP
#define WE_LOADER_EXCEPTIONS_HPP 1

#include <stdexcept>

namespace we
{
  namespace loader
  {
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

    class ModuleNotLoaded : public ModuleException {
    public:
      explicit
      ModuleNotLoaded(const std::string &a_module)
        : ModuleException("the desired module is not loaded: " + a_module, a_module) {}
      virtual ~ModuleNotLoaded() throw() {}
    };

    class ModuleLoadFailed : public ModuleException {
    public:
      explicit
      ModuleLoadFailed(const std::string &a_reason, const std::string &a_module, const std::string &a_file)
        : ModuleException(a_reason, a_module), file_(a_file) {}
      virtual ~ModuleLoadFailed() throw() {}

      const std::string &file() const { return file_; }
    private:
      std::string file_;
    };

    class ModuleInitFailed : public ModuleException {
    public:
      explicit
      ModuleInitFailed(const std::string &a_reason, const std::string &a_module, const std::string &a_file)
        : ModuleException(a_reason, a_module), file_(a_file) {}
      virtual ~ModuleInitFailed() throw() {}

      const std::string &file() const { return file_; }
    private:
      std::string file_;
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
