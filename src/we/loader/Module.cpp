// mirko.rahn@itwm.fraunhofer.de

#include <we/loader/Module.hpp>

#include <we/loader/exceptions.hpp>

#include <boost/format.hpp>

#include <exception>

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
          void (*function)(IModule*);
        };
      } func_ptr;

      func_ptr.symbol = dlsym (_dlhandle.handle(), "we_mod_initialize");

      if (func_ptr.function == nullptr)
      {
        throw std::logic_error
          ((boost::format ("Missing initialize function in %1%") % path).str());
      }

      func_ptr.function (this);
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
                      , drts::worker::context *info
                      , const expr::eval::context& input
                      , expr::eval::context& output
                      , std::map<std::string, void*> const& memory_buffer
                      )
    {
      const std::unordered_map<std::string, WrapperFunction>::const_iterator
        fun (call_table_.find (function));

      if (fun == call_table_.end())
      {
        throw function_not_found (name_, function);
      }
      else
      {
        (*fun->second)(info, input, output, memory_buffer);
      }
    }
    void Module::add_function (const std::string& name, WrapperFunction f)
    {
      if (! call_table_.emplace (name, f).second)
      {
        throw duplicate_function (name_, name);
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
