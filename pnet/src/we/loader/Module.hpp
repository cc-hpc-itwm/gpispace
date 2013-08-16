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
    class loader;

    class Module : public IModule {
    public:
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
        open (a_path, flags);
      }

      virtual ~Module() throw ()
      {
        try
        {
          close ();
        }
        catch (const std::exception & ex)
        {
          std::cerr << "E: **** module " << name()
                    << " from file " << path()
                    << " had errors during close: "
                    << ex.what()
                    << std::endl;
        }
        catch (...)
        {
          std::cerr << "E: **** module " << name()
                    << " from file " << path()
                    << " had unknow errors during close!"
                    << std::endl;
        }
      }

      inline void name (const std::string & a_name) { name_ = a_name; }
      inline const std::string &name() const { return name_; }

      inline const std::string &path() const { return path_; }

      void * state (void)
      {
        return state_;
      }

      const void * state (void) const
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
                       , const expr::eval::context &input
                       , expr::eval::context &output
                       )
      {
        return call (function, input, output);
      }

      void call( const std::string &function
               , const expr::eval::context &input
               , expr::eval::context &output
               )
      {
        call_table_t::const_iterator fun = call_table_.find(function);
        if (fun == call_table_.end())
        {
          throw FunctionNotFound ( name(), function );
        }
        else
        {
          (*(fun->second.first))(state(), input, output);
        }
      }

      void add_function( const std::string &function
                       , WrapperFunction f
                       )
      {
        std::pair<call_table_t::iterator, bool> insertPosition
          = call_table_.insert
          ( std::make_pair ( function
                           , std::make_pair (f, param_names_list_t())
                           )
          );
        if (! insertPosition.second) {
          throw DuplicateFunction(name(), function);
        }
      }

      void init (loader *) throw (ModuleException)
      {
        if (! handle_)
        {
          throw ModuleInitFailed
            ("initialization of module " + name() + " from " + path() + " failed: handle is 0", name(), path());
        }

        InitializeFunction initialize (NULL);
        {
          struct
          {
            union
            {
              void * symbol;
              InitializeFunction function;
            };
          } func_ptr;

          func_ptr.symbol = dlsym(handle_, "we_mod_initialize");
          initialize = func_ptr.function;
        }

        if (initialize != NULL)
        {
          try {
            const unsigned int LOADER_VERSION (1U);

            initialize( this, LOADER_VERSION );
          } catch (const std::exception &ex) {
            throw ModuleInitFailed("error during mod-init function: " + std::string(ex.what()), name(), path());
          } catch (...) {
            throw ModuleInitFailed("unknown error during mod-init function", name(), path());
          }
        }
      }

    private:
      void open (const std::string & a_path, int flags = RTLD_NOW | RTLD_GLOBAL)
      {
        if (handle_)
          return;

        handle_ = dlopen(a_path.c_str(), flags);
        if (! handle_)
        {
          throw ModuleLoadFailed( std::string("could not load module '")
                                + name()
                                + "' from '"+ a_path + "': "
                                + std::string(dlerror())
                                , name()
                                , a_path
                                );
        }

        dlerror();

        try
        {
          init (0);
        }
        catch (...)
        {
          close();
          throw;
        }
      }

      void close ()
      {
        if (! handle_)
          return;

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

        if (finalize != NULL)
        {
          try
          {
            finalize( this );
          }
          catch (...)
          {
            dlclose(handle_);
            handle_ = 0;
            throw;
          }
        }

        dlclose(handle_);
        handle_ = 0;
      }

    private:
      Module(const Module&);
      Module &operator=(const Module &);

      std::string name_;
      std::string path_;
      void* handle_;
      call_table_t call_table_;
      void *state_;
    };
  }
}

#endif
