#ifndef WE_LOADER_MODULE_HPP
#define WE_LOADER_MODULE_HPP 1

#include <iostream>
#include <boost/unordered_map.hpp>
#include <string>

#include <we/loader/types.hpp>
#include <we/loader/IModule.hpp>
#include <we/loader/exceptions.hpp>

#include <dlfcn.h>

namespace we
{
  namespace loader
  {
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
      typedef void* handle_t;

      typedef boost::unordered_map<std::string, parameterized_function_t> call_table_t;
    public:
      Module( const std::string & a_name
            , const std::string & a_path
            , int flags = RTLD_NOW | RTLD_GLOBAL
            )
        : name_(a_name)
        , path_(a_path)
        , handle_(0)
        , call_table_()
        , state_(0)
      {
        handle_ = open (a_path, flags);
      }

      virtual ~Module() throw ()
      {
        try
        {
          close ();
        }
        catch (const std::exception & ex)
        {
          std::cerr << "E: **** module " << name() << " had errors during close: " << ex.what() << std::endl;
        }
      }

      inline void name (const std::string & a_name) { name_ = a_name; }
      inline const std::string &name() const { return name_; }

      void * state (void)
      {
        return state_;
      }

      void * state (void * new_state)
      {
        void * old_state (state_);
        state_ = new_state;
        return old_state;
      }

      void operator () ( const std::string &function
                       , const input_t &input
                       , output_t &output
                       ) throw ( FunctionNotFound
                               , BadFunctionArgument
                               , FunctionException
                               , std::exception
                               )
      {
        return call (function, input, output);
      }

      void call( const std::string &function
               , const input_t &input
               , output_t &output
               ) throw ( FunctionNotFound
                             , BadFunctionArgument
                             , FunctionException
                             , std::exception
                             )
      {
        call_table_t::const_iterator fun = call_table_.find(function);
        if (fun == call_table_.end())
        {
          throw FunctionNotFound ( name(), function );
        }
        else
        {

#if 0
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

          // hopefully safe to call now
          (*(fun->second.first))(state(), input, output);
        }
      }

      void add_function ( const std::string &function
                        , WrapperFunction f
                        ) throw ( DuplicateFunction
                                , FunctionException
                                )
      {
        return add_function(function, f, param_names_list_t());
      }

      void add_function( const std::string &function
                       , WrapperFunction f
                       , const param_names_list_t &parameters
                       ) throw ( DuplicateFunction
                               , FunctionException
                               )
      {
#if 0
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
      handle_t open (const std::string & a_path, int flags = RTLD_NOW | RTLD_GLOBAL)
      {
        handle_t handle = dlopen(a_path.c_str(), flags);
        if (! handle)
        {
          throw ModuleLoadFailed( std::string("could not load module '")
                                + name()
                                + "' from '"+ a_path + "': "
                                + std::string(dlerror())
                                , name()
                                , a_path
                                );
        }

        // clear any errors
        dlerror();

        InitializeFunction init (NULL);
        {
          struct
          {
            union
            {
              void * symbol;
              InitializeFunction function;
            };
          } func_ptr;

          func_ptr.symbol = dlsym(handle, "we_mod_initialize");
          init = func_ptr.function;
        }

        if (init != NULL)
        {
          try {
            const unsigned int LOADER_VERSION (1U);

            init( this, LOADER_VERSION );
          } catch (const std::exception &ex) {
            dlclose(handle);
            throw ModuleLoadFailed("error during mod-init function: " + std::string(ex.what()), name(), a_path);
          } catch (...) {
            dlclose(handle);
            throw ModuleLoadFailed("unknown error during mod-init function", name(), a_path);
          }
        }

        return handle;
      }

      void close ()
      {
        if (! handle_)
          return;

        // clear any errors
        dlerror();

        FinalizeFunction finalize (NULL);
        {
          struct
          {
            union
            {
              void * symbol;
              FinalizeFunction function;
            };
          } func_ptr;

          func_ptr.symbol = dlsym(handle_, "we_mod_finalize");
          finalize = func_ptr.function;
        }

        try
        {
          if (finalize != NULL)
          {
            finalize( this );
          }
        }
        catch (...)
        {
          dlclose(handle_);
          handle_ = 0;
          throw;
        }
      }

    private:
      // currently we do not want copy operations, thus, define them, but don't
      // implement them
      Module(const Module&);
      Module &operator=(const Module &);

      std::string name_;
      std::string path_;
      handle_t handle_;
      call_table_t call_table_;
      void *state_;
    };

    inline std::ostream &operator<<(std::ostream &os, const Module &mod)
    {
      mod.writeTo(os);
      return os;
    }
  }
}

#endif
