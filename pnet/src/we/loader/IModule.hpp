// alexander.petry@itwm.fraunhofer.de

#ifndef WE_LOADER_IMODULE_HPP
#define WE_LOADER_IMODULE_HPP 1

#include <we/expr/eval/context.hpp>
#include <we/loader/api-guard.hpp>

#include <list>
#include <string>

namespace we
{
  namespace loader
  {
    typedef void (*WrapperFunction)( void*
                                   , const expr::eval::context&
                                   , expr::eval::context&
                                   );
    typedef std::list<std::string> param_names_list_t;
    typedef std::pair< WrapperFunction
                     , param_names_list_t
                     > parameterized_function_t;

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
