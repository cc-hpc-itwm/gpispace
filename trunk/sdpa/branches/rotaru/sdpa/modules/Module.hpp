#ifndef SDPA_MODULES_MODULE_HPP
#define SDPA_MODULES_MODULE_HPP 1

#include <string>
#include <list>

#include <sdpa/memory.hpp>
#include <sdpa/wf/Activity.hpp>
#include <sdpa/modules/exceptions.hpp>

namespace sdpa {
namespace modules {

#define SDPA_MOD_INIT_START(modname) extern "C" { void sdpa_mod_init(Module *mod) { mod->name(#modname);
#define SDPA_REGISTER_FUN(fun) mod->add_function(#fun, &fun)
#define SDPA_MOD_INIT_END(modname) }}

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
      typedef shared_ptr<Module> ptr_t;

      typedef void* handle_t;
      typedef void (*InitFunction)(Module*);

      typedef sdpa::wf::parameters_t data_t;
      typedef void (*GenericFunction)(data_t&);

      typedef std::map<std::string, GenericFunction> call_table_t;
    public:
      /* the init function has to set should set the module's name! */
      explicit
      Module(handle_t a_handle)
        : name_("")
        , handle_(a_handle)
        , call_table_()
      { }

      ~Module()
      {
      }

      inline const std::string &name() const { return name_; }
      inline void name(const std::string &a_name) { name_ = a_name; }

      inline const handle_t &handle() { return handle_; }

      void call(const std::string &function, data_t &data) const throw (FunctionNotFound, BadFunctionArgument, FunctionException, std::exception)
      {
        call_table_t::const_iterator f = call_table_.find(function);
        if (f == call_table_.end())
        {
          throw FunctionNotFound(name(), function);
        }
        else
        {
          (*(f->second))(data);
        }
      }

      void add_function(const std::string &function, GenericFunction f) throw (DuplicateFunction, FunctionException)
      {
        std::pair<call_table_t::iterator, bool> insertPosition = call_table_.insert(std::make_pair(function, f));
        if (! insertPosition.second) {
          throw DuplicateFunction(name(), function);
        }
      }
    private:
      // currently we do not want copy operations, thus, define them, but don't
      // implement them
      Module(const Module&);
      Module &operator=(const Module &);

      std::string name_;
      handle_t handle_;
      call_table_t call_table_;
  };
}}

#endif
