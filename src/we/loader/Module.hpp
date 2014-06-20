#ifndef WE_LOADER_MODULE_HPP
#define WE_LOADER_MODULE_HPP 1

#include <we/loader/IModule.hpp>

#include <boost/utility.hpp>

#include <string>
#include <unordered_map>

#include <dlfcn.h>

namespace we
{
  namespace loader
  {
    class Module : public IModule, boost::noncopyable
    {
    public:
      Module ( const std::string& path
             , int flags = RTLD_NOW | RTLD_GLOBAL
             );
      virtual ~Module() throw();

      virtual void name (const std::string&) override;
      const std::string &name() const;
      const std::string &path() const;

      void call ( const std::string& f
                , drts::worker::context *context
                , const expr::eval::context& in
                , expr::eval::context& out
                );

      virtual void add_function (const std::string&, WrapperFunction) override;

    private:
      std::string name_;
      std::string path_;

      class dlhandle
      {
      public:
        dlhandle (std::string const& path, int flags);
        ~dlhandle();
        void* handle() const;
      private:
        void* _handle;
      } _dlhandle;
      std::unordered_map<std::string, WrapperFunction> call_table_;
    };
  }
}

#endif
