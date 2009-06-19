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
    ModuleException(const std::string &reason, const std::string &module)
    : sdpa::SDPAException(reason), module_(module) {}

    virtual ~ModuleException() throw() {}
    const std::string &module() const { return module_; }
  private:
    std::string module_;
  };

  class ModuleNotLoaded : public ModuleException {
  public:
    explicit
    ModuleNotLoaded(const std::string &module)
      : ModuleException("the desired module is not loaded", module) {}
    virtual ~ModuleNotLoaded() throw() {}
  };

  class ModuleLoadFailed : public ModuleException {
  public:
    explicit
    ModuleLoadFailed(const std::string &reason, const std::string &module, const std::string &file)
      : ModuleException(reason, module), file_(file) {}
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
    FunctionException(const std::string &reason, const std::string &module, const std::string &function)
      : ModuleException(reason, module), function_(function) {}

    virtual ~FunctionException() throw() {}

    const std::string &function() const { return function_; }
  private:
    std::string function_;
  };

  class FunctionNotFound : public FunctionException {
  public:
    FunctionNotFound(const std::string &module, const std::string &function)
      : FunctionException("function could not be found", module, function) {}
    virtual ~FunctionNotFound() throw() {}
  };

  class DuplicateFunction : public FunctionException {
  public:
    DuplicateFunction(const std::string &module, const std::string &function)
      : FunctionException("duplicate function detected", module, function) {}
    virtual ~DuplicateFunction() throw() {}
  };

  class BadFunctionArgument : public FunctionException {
  public:
    BadFunctionArgument(const std::string &module, const std::string &function,
                        const std::string &expected, const std::string &actual,
                        const std::string &value="")
      : FunctionException("bad function argument", module, function),
        expected_(expected), actual_(actual), value_(value) {}
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
