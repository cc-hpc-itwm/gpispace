#ifndef SDPA_MODULES_MODULE_HPP
#define SDPA_MODULES_MODULE_HPP 1

#include <fhglog/fhglog.hpp>

#include <iostream>
#include <string>
#include <list>

#include <sdpa/memory.hpp>
#include <sdpa/modules/types.hpp>
#include <sdpa/modules/IModule.hpp>
#include <sdpa/modules/exceptions.hpp>

namespace sdpa {
namespace modules {
  /**
    @brief
    This class describes a loaded module, it keeps a mapping of function names
    to actual functions.

    All functions follow the same interface: func(const TokenList&,
    TokenList&). The const TokenList reference represents input tokens and
    cannot be modified, the latter list represents an output list and the
    function is able to modify it.
   */
  class Module : public IModule {
    public:
      typedef shared_ptr<Module> ptr_t;

      typedef void* handle_t;

      typedef std::map<std::string, parameterized_function_t> call_table_t;
    public:
      /* the init function has to set should set the module's name! */
      explicit
      Module(handle_t a_handle)
        : name_("")
        , handle_(a_handle)
        , call_table_()
      { }

      virtual ~Module() throw () {}

      inline const std::string &name() const { return name_; }
      inline void name(const std::string &a_name) { name_ = a_name; }

      inline const handle_t &handle() { return handle_; }

      void call(const std::string &function, data_t &data, const bool keep_going = false) const throw (FunctionNotFound, BadFunctionArgument, FunctionException, std::exception)
      {
        call_table_t::const_iterator fun = call_table_.find(function);
        if (fun == call_table_.end())
        {
          throw FunctionNotFound(name(), function);
        }
        else
        {

#if defined(ENABLE_SDPA_PARAM_CHECKS) || !defined(NDEBUG)
          DLOG(TRACE, "checking passed data against my expected parameters...");
          const param_names_list_t &expected_input = fun->second.second;
          std::string missing;
          for (param_names_list_t::const_iterator exp_inp(expected_input.begin()); exp_inp != expected_input.end(); ++exp_inp)
          {
            // locate if the expected input parameter is in the data also
            data_t::const_iterator act_inp(data.find(*exp_inp));
            if (act_inp == data.end())
            {
              DLOG(ERROR, name() << "." << function << " without required parameter " << *exp_inp);
              missing += ", " + *exp_inp;
            }
          }

          if (! keep_going && (! missing.empty()))
          {
            throw MissingFunctionArgument(name(), function, missing.substr(2)); // substr() -> remove leading ", "
          }
#endif

          (*(fun->second.first))(data); // hopefully safe to call now
        }
      }

      void add_function(const std::string &function, GenericFunction f) throw (DuplicateFunction, FunctionException)
      {
        return add_function(function, f, param_names_list_t());
      }
      void add_function(const std::string &function, GenericFunction f, const param_names_list_t &parameters) throw (DuplicateFunction, FunctionException)
      {
#ifndef NDEBUG
        {
          std::ostringstream ostr;
          param_names_list_t::const_iterator exp_inp(parameters.begin());
          while (exp_inp != parameters.end())
          {
            ostr << *exp_inp;
            ++exp_inp;
            if (exp_inp != parameters.end()) ostr << ", ";
          }
          DLOG(DEBUG, "adding function " << function << "( " << ostr.str() << " )");
        }
#endif
        std::pair<call_table_t::iterator, bool> insertPosition = call_table_.insert(std::make_pair(function, std::make_pair(f, parameters)));
        if (! insertPosition.second) {
          throw DuplicateFunction(name(), function);
        }
      }

      void writeTo(std::ostream &os) const
      {
        os << "{mod, " << name() << ", ";
        os << "[ ";
        {
          call_table_t::const_iterator fun(call_table_.begin());
          while (fun != call_table_.end())
          {
            os << fun->first; // function name
            os << "( ";

            param_names_list_t::const_iterator exp_inp(fun->second.second.begin());
            while (exp_inp != fun->second.second.end())
            {
              os << *exp_inp;
              ++exp_inp;
              if (exp_inp != fun->second.second.end()) os << ", ";
            }

            os << " )";

            fun++;
            if (fun != call_table_.end()) os << ", ";
          }
        }

        os << " ]";
        os << "}";
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

inline std::ostream &operator<<(std::ostream &os, const sdpa::modules::Module &mod)
{
  mod.writeTo(os);
  return os;
}

#endif
