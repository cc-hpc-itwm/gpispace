// mirko.rahn@itwm.fraunhofer.de

#ifndef _WE_EXPR_EXCEPTION_HPP
#define _WE_EXPR_EXCEPTION_HPP

#include <boost/format.hpp>

#include <stdexcept>

namespace expr
{
  namespace exception
  {
    namespace parse
    {
      class exception : public std::runtime_error
      {
      public:
        const std::size_t eaten;
        exception (const std::string&, const std::size_t);
      };

      class expected : public exception
      {
      public:
        expected (const std::string&, const std::size_t);
      };

      class misplaced : public exception
      {
      public:
        misplaced (const std::string&, const std::size_t);
      };

      class unterminated : public exception
      {
      public:
        unterminated ( const std::string&
                     , const std::size_t open
                     , const std::size_t k
                     );
      };

      class missing : public exception
      {
      public:
        missing (const std::string&, const std::size_t);
      };
    }

    namespace eval
    {
      class divide_by_zero : public std::runtime_error
      {
      public:
        divide_by_zero();
      };

      class type_error : public std::runtime_error
      {
      public:
        type_error (const std::string&);
        type_error (const boost::format&);
      };

      class negative_exponent : public std::runtime_error
      {
      public:
        negative_exponent();
      };
    }
  }
}

#endif
