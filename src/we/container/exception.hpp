// mirko.rahn@itwm.fhg.de

#ifndef _WE_CONTAINER_EXCEPTION_HPP
#define _WE_CONTAINER_EXCEPTION_HPP 1

#include <stdexcept>
#include <string>

namespace we
{
  namespace container
  {
    namespace exception
    {
      class no_such : public std::runtime_error
      {
      public:
        explicit no_such (const std::string & msg)
          : std::runtime_error (msg)
        {}
        ~no_such() throw() {}
      };

      class already_there : public std::runtime_error
      {
      public:
        explicit already_there (const std::string & msg)
          : std::runtime_error (msg)
        {}
        ~already_there() throw() {}
      };
    }
  }
}

#endif
