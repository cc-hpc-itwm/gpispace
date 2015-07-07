#pragma once

#include <we/loader/wrapper.hpp>

#include <boost/utility.hpp>

#include <string>
#include <unordered_map>
#include <map>

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

      const std::string &path() const;

      void call ( const std::string& f
                , drts::worker::context *context
                , const expr::eval::context& in
                , expr::eval::context& out
                , std::map<std::string, void*> const& memory_buffer
                ) const;

      virtual void add_function (const std::string&, WrapperFunction) override;

    private:
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
