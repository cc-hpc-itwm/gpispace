// mirko.rahn@itwm.fraunhofer.de

#include <we/loader/Module.hpp>

#include <we/loader/exceptions.hpp>

#include <fhglog/LogMacros.hpp>

#include <iostream>

namespace we
{
  namespace loader
  {
    Module::Module ( const std::string& path
                   , int flags
                   )
      : name_()
      , path_ (path)
      , _dlhandle (path, flags)
      , call_table_()
    {
      struct
      {
        union
        {
          void * symbol;
          void (*function)(IModule*, unsigned int);
        };
      } func_ptr;

      func_ptr.symbol = dlsym (_dlhandle.handle(), "we_mod_initialize");

      if (func_ptr.function != NULL)
      {
        const unsigned int LOADER_VERSION (1U);

        func_ptr.function (this, LOADER_VERSION);
      }

      MLOG (TRACE, "loaded module " << name_ << " from " << path_);
    }
    Module::~Module() throw()
    {
      MLOG (TRACE, "unloading " << name_);

      try
      {
        struct
        {
          union
          {
            void * symbol;
            void (*function) (IModule*);
          };
        } func_ptr;

        func_ptr.symbol = dlsym (_dlhandle.handle(), "we_mod_finalize");

        if (func_ptr.function != NULL)
        {
          func_ptr.function (this);
        }
      }
      catch (const std::exception& ex)
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
                  << " had unknown errors during close!"
                  << std::endl;
      }
    }
    void Module::name (const std::string& a_name)
    {
      name_ = a_name;
    }
    const std::string &Module::name() const
    {
      return name_;
    }
    const std::string &Module::path() const
    {
      return path_;
    }
    void Module::call ( const std::string& function
                      , gspc::drts::context *info
                      , const expr::eval::context& input
                      , expr::eval::context& output
                      )
    {
      const boost::unordered_map< std::string
                                , parameterized_function_t
                                >::const_iterator
        fun (call_table_.find (function));

      if (fun == call_table_.end())
      {
        throw FunctionNotFound (name(), function);
      }
      else
      {
        (*(fun->second.first))(info, input, output);
      }
    }
    void Module::add_function (const std::string& name, WrapperFunction f)
    {
      if (! call_table_.insert
            ( std::make_pair (name, std::make_pair (f, param_names_list_t()))
            ).second
         )
      {
        throw DuplicateFunction (name_, name);
      }
    }

    Module::dlhandle::dlhandle ( std::string const& path
                               , int flags
                               )
      : _handle (dlopen (path.c_str(), flags))
    {
      if (!_handle)
      {
        throw module_load_failed (path, dlerror());
      }
    }
    Module::dlhandle::~dlhandle()
    {
      dlclose (_handle);
    }
    void* Module::dlhandle::handle() const
    {
      return _handle;
    }
  }
}
