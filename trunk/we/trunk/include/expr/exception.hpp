// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EXCEPTION_HPP
#define _EXPR_EXCEPTION_HPP

#include <util/show.hpp>

#include <stdexcept>

namespace expr
{
  class exception : public std::runtime_error
  {
  public:
    unsigned int eaten;
    exception (const std::string & msg, const unsigned int k)
      : std::runtime_error("parse error [" + show(k) + "]: " + msg) 
      , eaten (k)
    {}
  };

  class expected : public exception
  {
  public:
    expected (const std::string & what, const unsigned int k)
      : exception ("expected '" + what + "'", k) {}
  };

  class divide_by_zero : public std::runtime_error
  {
  public:
    explicit divide_by_zero () : std::runtime_error ("divide by zero") {};
  };
}

#endif
