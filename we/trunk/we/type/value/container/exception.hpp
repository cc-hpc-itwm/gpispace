// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_TYPE_VALUE_CONTAINER_EXCEPTION_HPP
#define _WE_TYPE_VALUE_CONTAINER_EXCEPTION_HPP 1

#include <stdexcept>

namespace value
{
  namespace container
  {
    namespace exception
    {
      class missing_binding : public std::runtime_error
      {
      public:
        explicit missing_binding (const std::string & key)
          : std::runtime_error ("missing binding for: ${" + key + "}")
        {};
      };
    }
  }
}

#endif
