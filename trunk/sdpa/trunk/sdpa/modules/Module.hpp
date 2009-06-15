#ifndef SDPA_MODULES_MODULE_HPP
#define SDPA_MODULES_MODULE_HPP 1

#include <string>
#include <list>

#include <sdpa/memory.hpp>
#include <sdpa/Token.hpp>
#include <sdpa/modules/exceptions.hpp>

namespace sdpa {
namespace modules {

#define SDPA_REGISTER_NAMED_FUN(mod_ptr, name, fun) (mod_ptr)->add_function(name, &fun)
#define SDPA_REGISTER_FUN(mod_ptr, fun) SDPA_REGISTER_NAMED_FUN(mod_ptr, #fun, fun)

  /**
    @brief
    This class describes a loaded module, it keeps a mapping of function names
    to actual functions.

    All functions follow the same interface: func(const TokenList&,
    TokenList&). The const TokenList reference represents input tokens and
    cannot be modified, the latter list represents an output list and the
    function is able to modify it.
   */
  class Module {
    public:
      typedef sdpa::shared_ptr<Module> Ptr;

      typedef void* handle_t;
      typedef void (*InitFunction)(Module*);

      typedef std::list<sdpa::Token> data_t;
      typedef data_t input_data_t;
      typedef data_t output_data_t;

      typedef void (*GenericFunction)(const input_data_t&, output_data_t&);

      typedef std::map<std::string, GenericFunction> call_table_t;
    public:
      Module(const std::string &name, handle_t handle);

      ~Module();

      const std::string &name() const { return name_; }
      handle_t handle() { return handle_; }

      void call(const std::string &function, const input_data_t&, output_data_t&) const throw (FunctionNotFound, BadFunctionArgument, FunctionException);
      void add_function(const std::string &function, GenericFunction) throw (DuplicateFunction, FunctionException);
    private:
      std::string name_;
      handle_t handle_;
      call_table_t call_table_;
  };
}}

#endif
