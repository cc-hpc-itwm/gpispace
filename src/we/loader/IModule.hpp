// alexander.petry@itwm.fraunhofer.de

#ifndef WE_LOADER_IMODULE_HPP
#define WE_LOADER_IMODULE_HPP 1

#include <we/expr/eval/context.hpp>
#include <we/loader/api-guard.hpp>

#include <drts/worker/context_fwd.hpp>

#include <list>
#include <string>

namespace we
{
  namespace loader
  {
    typedef void (*WrapperFunction)( drts::worker::context *
                                   , const expr::eval::context&
                                   , expr::eval::context&
                                   );

    class IModule
    {
    public:
      virtual ~IModule() {}

      virtual void name (const std::string &name) = 0;
      virtual void add_function (const std::string&, WrapperFunction) = 0;
    };
  }
}

#endif
