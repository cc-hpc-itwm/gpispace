// mirko.rahn@itwm.fraunhofer.de

#include <we/loader/Module.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/format.hpp>

namespace we
{
  namespace loader
  {
    Module::Module ( const std::string& name
                   , const std::string& path
                   , int flags
                   )
      : name_ (name)
      , path_ (path)
      , handle_ (dlopen (path_.c_str(), flags))
      , call_table_()
    {
      if (!handle_)
      {
        throw ModuleLoadFailed
          ( ( boost::format ("could not load module '%1%' from '%2%': %3%")
            % name_
            % path_
            % dlerror()
            ).str()
          , name_
          , path_
          );
      }

      dlerror();

      try
      {
        struct
        {
          union
          {
            void * symbol;
            void (*function)(IModule*, unsigned int);
          };
        } func_ptr;

        func_ptr.symbol = dlsym (handle_, "we_mod_initialize");

        if (func_ptr.function != NULL)
        {
          try
          {
            const unsigned int LOADER_VERSION (1U);

            func_ptr.function (this, LOADER_VERSION);
          }
          catch (const std::exception &ex)
          {
            throw ModuleInitFailed ("error during mod-init function: " + std::string(ex.what()), name_, path_);
          }
          catch (...)
          {
            throw ModuleInitFailed ("unknown error during mod-init function", name_, path_);
          }
        }
      }
      catch (...)
      {
        close();
        throw;
      }

      MLOG (TRACE, "loaded module " << name_ << " from " << path_);
    }
    Module::~Module() throw()
    {
      MLOG (TRACE, "unloading " << name_);

      try
      {
        close ();
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
                  << " had unknow errors during close!"
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
    void Module::close()
    {
      if (!handle_)
        return;

      dlerror();

      struct
      {
        union
        {
          void * symbol;
          void (*function) (IModule*);
        };
      } func_ptr;

      func_ptr.symbol = dlsym (handle_, "we_mod_finalize");

      if (func_ptr.function != NULL)
      {
        try
        {
          func_ptr.function (this);
        }
        catch (...)
        {
          dlclose (handle_);
          handle_ = 0;
          throw;
        }
      }

      dlclose (handle_);
      handle_ = 0;
    }
  }
}
