// mirko.rahn@itwm.fraunhofer.de

#ifndef _EXPR_EXCEPTION_HPP
#define _EXPR_EXCEPTION_HPP

#include <fhg/util/show.hpp>

#include <stdexcept>

namespace expr
{
  namespace exception
  {
    class strange : public std::runtime_error
    {
    public:
      explicit strange (const std::string & what)
        : std::runtime_error ("STRANGE! " + what)
      {}
    };

    namespace parse
    {
      class exception : public std::runtime_error
      {
      public:
        const std::size_t eaten;
        exception (const std::string & msg, const std::size_t k)
          : std::runtime_error("parse error [" + fhg::util::show(k) + "]: " + msg)
          , eaten (k)
        {}
      };

      class expected : public exception
      {
      public:
        expected (const std::string & what, const std::size_t k)
          : exception ("expected " + what, k) {}
      };

      class misplaced : public exception
      {
      public:
        misplaced (const std::string & what, const std::size_t k)
          : exception ( "misplaced " + what + ", operator expected"
                      , k - what.length()
                      ) {}
      };

      class unterminated : public exception
      {
      public:
        unterminated ( const std::string & what
                     , const std::size_t open
                     , const std::size_t k
                     )
          : exception ( "unterminated " + what + ", opened at: " + fhg::util::show (open)
                      , k
                      ) {}
      };

      class missing : public exception
      {
      public:
        missing (const std::string & what, const std::size_t k)
          : exception ("missing " + what, k) {}
      };
    }

    namespace eval
    {
      class divide_by_zero : public std::runtime_error
      {
      public:
        divide_by_zero () : std::runtime_error ("divide by zero") {};
      };

      class type_error : public std::runtime_error
      {
      public:
        type_error (const std::string & msg)
          : std::runtime_error ("type error: " + msg)
        {}
      };

      class negative_exponent : public std::runtime_error
      {
      public:
        negative_exponent() : std::runtime_error ("negative exponent") {};
      };
    }
  }
}

#endif
