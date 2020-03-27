#pragma once

#include <we/loader/IModule.hpp>

#include <util-generic/dynamic_linking.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/utility.hpp>

#include <string>
#include <unordered_map>
#include <map>

namespace we
{
  namespace loader
  {
    class Module : public IModule, boost::noncopyable
    {
    public:
      Module (boost::filesystem::path const& path);

      void call ( const std::string& f
                , drts::worker::context *context
                , const expr::eval::context& in
                , expr::eval::context& out
                , std::map<std::string, void*> const& memory_buffer
                ) const;

      virtual void add_function (const std::string&, WrapperFunction) override;

    private:
      boost::filesystem::path path_;

      fhg::util::scoped_dlhandle _dlhandle;
      std::unordered_map<std::string, WrapperFunction> call_table_;
    };
  }
}
