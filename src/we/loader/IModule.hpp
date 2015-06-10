#pragma once

#include <we/expr/eval/context.hpp>
#include <we/loader/api-guard.hpp>

#include <drts/worker/context_fwd.hpp>

#include <map>
#include <string>

namespace we
{
  namespace loader
  {
    typedef void (*WrapperFunction)( drts::worker::context *
                                   , const expr::eval::context&
                                   , expr::eval::context&
                                   , std::map<std::string, void*> const&
                                   );

    class IModule
    {
    public:
      virtual ~IModule() {}

      virtual void add_function (const std::string&, WrapperFunction) = 0;
    };
  }
}
