// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EXCEPTION_HPP
#define _EXPR_EXCEPTION_HPP

#include <we/util/show.hpp>

#include <stdexcept>

namespace expr
{
  class exception : public std::runtime_error
  {
  public:
    const unsigned int eaten;
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

  class misplaced : public exception
  {
  public:
    misplaced (const std::string & what, const unsigned int k)
      : exception ( "misplaced " + what + ", operator expected"
                  , k - what.length()
                  ) {}
  };

  class divide_by_zero : public std::runtime_error
  {
  public:
    divide_by_zero () : std::runtime_error ("divide by zero") {};
  };
}

#endif
