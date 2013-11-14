// mirko.rahn@itwm.fraunhofer.de

#include <we/loader/Module.hpp>

namespace we
{
  namespace loader
  {
    Module::Module ( const std::string& a_name
                   , const std::string& a_path
                   , int flags
                   )
      : name_ (a_name)
      , path_ (a_path)
      , handle_(0)
      , call_table_()
    {
      open (a_path, flags);
    }
    Module::~Module() throw()
    {
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
    void Module::init (loader*) throw (ModuleException)
    {
      if (! handle_)
      {
        throw ModuleInitFailed
          ("initialization of module " + name() + " from " + path() + " failed: handle is 0", name(), path());
      }

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
          throw ModuleInitFailed ("error during mod-init function: " + std::string(ex.what()), name(), path());
        }
        catch (...)
        {
          throw ModuleInitFailed ("unknown error during mod-init function", name(), path());
        }
      }
    }
    void Module::open (const std::string& a_path, int flags)
    {
      if (handle_)
        return;

      handle_ = dlopen(a_path.c_str(), flags);
      if (!handle_)
      {
        throw ModuleLoadFailed ( std::string("could not load module '")
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
