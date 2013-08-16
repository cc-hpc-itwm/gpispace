#ifndef WE_LOADER_TYPES_HPP
#define WE_LOADER_TYPES_HPP 1

#include <we/expr/eval/context.hpp>

#include <list>

namespace we
{
  namespace loader
  {
    class IModule;

    typedef void (*WrapperFunction)(void*, const expr::eval::context&, expr::eval::context&);

    typedef std::list<std::string> param_names_list_t;
    typedef std::pair< WrapperFunction
                     , param_names_list_t
                     > parameterized_function_t;
  }
}

#endif
