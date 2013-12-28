#ifndef WE_LOADER_MODULE_HPP
#define WE_LOADER_MODULE_HPP 1

#include <we/loader/IModule.hpp>
#include <we/loader/exceptions.hpp>

#include <boost/unordered_map.hpp>
#include <boost/utility.hpp>

#include <iostream>
#include <string>

#include <dlfcn.h>

namespace we
{
  namespace loader
  {
    class loader;

    class Module : public IModule, boost::noncopyable
    {
    public:
      Module ( const std::string& name
             , const std::string& path
             , int flags = RTLD_NOW | RTLD_GLOBAL
             );
      virtual ~Module() throw();

      void name (const std::string&);
      const std::string &name() const;
      const std::string &path() const;

      void call ( const std::string& f
                , gspc::drts::context *context
                , const expr::eval::context& in
                , expr::eval::context& out
                );

      void add_function (const std::string&, WrapperFunction);

    private:
      void init (loader*) throw (ModuleException);

      void close();

      std::string name_;
      std::string path_;
      void* handle_;
      boost::unordered_map<std::string, parameterized_function_t> call_table_;
    };
  }
}

#endif
