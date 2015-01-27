#ifndef FHG_UTIL_NEST_EXCEPTIONS_HPP
#define FHG_UTIL_NEST_EXCEPTIONS_HPP

#include <exception>
#include <type_traits>

namespace fhg
{
  namespace util
  {
    template<typename Exception, typename... ExceptionArgs, typename Fun>
      auto nest_exceptions (Fun&& fun, ExceptionArgs&&... exception_args)
      -> decltype (fun())
    {
      try
      {
        return fun();
      }
      catch (...)
      {
        std::throw_with_nested
          (Exception (std::forward<ExceptionArgs> (exception_args)...));
      }
    }
  }
}

#endif
