// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EXCEPTION_HPP
#define _EXPR_EXCEPTION_HPP

#include <stdexcept>

namespace expr
{
  class exception : public std::runtime_error
  {
  public:
    explicit exception (const std::string & msg)
      : std::runtime_error("parse error: " + msg) {}
  };

  class expected : public exception
  {
  public:
    explicit expected (const std::string & what)
      : exception ("expected '" + what + "'") {}
  };

  class divide_by_zero : public std::runtime_error
  {
  public:
    explicit divide_by_zero () : std::runtime_error ("divide by zero") {};
  };
}

#endif
