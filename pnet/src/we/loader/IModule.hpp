// alexander.petry@itwm.fraunhofer.de

#ifndef WE_LOADER_IMODULE_HPP
#define WE_LOADER_IMODULE_HPP 1

#include <string>

#include <we/loader/types.hpp>
#include <we/loader/api-guard.hpp>

namespace we
{
  namespace loader
  {
    class IModule
    {
    public:
      virtual ~IModule() throw () {}

      virtual void name (const std::string &name) = 0;
      virtual void add_function (const std::string&, WrapperFunction) = 0;
      virtual void* state (void*) = 0;
      virtual void* state (void) = 0;
    };
  }
}

#endif
