#ifndef SDPA_MODULES_EXCEPTIONS_HPP
#define SDPA_MODULES_EXCEPTIONS_HPP 1

#include <sdpa/SDPAException.hpp>

namespace sdpa {
namespace modules {
  /**
    All exceptions related to a module.
    */
  class ModuleException : public sdpa::SDPAException {
  public:
    ModuleException(const std::string &a_reason, const std::string &a_module)
    : sdpa::SDPAException(a_reason), module_(a_module) {}

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

  class MissingFunctionArgument : public FunctionException {
  public:
    MissingFunctionArgument(const std::string &a_module
                          , const std::string &a_function
                          , const std::string &expected_arguments)
      : FunctionException("missing argument(s): " + expected_arguments, a_module, a_function)
      , arguments_(expected_arguments)
    { }

    virtual ~MissingFunctionArgument() throw() {}

    const std::string &arguments() const { return arguments_; }
  private:
    std::string arguments_;
  };

  class BadFunctionArgument : public FunctionException {
  public:
    BadFunctionArgument(const std::string &a_module, const std::string &a_function,
                        const std::string &a_expected, const std::string &a_actual,
                        const std::string &a_value="")
      : FunctionException("bad function argument", a_module, a_function),
        expected_(a_expected), actual_(a_actual), value_(a_value) {}
    virtual ~BadFunctionArgument() throw() {}

    const std::string &expected() const { return expected_; }
    const std::string &actual() const { return actual_; }
    const std::string &value() const { return value_; }
  private:
    std::string expected_;
    std::string actual_;
    std::string value_;
  };
}}

#endif
