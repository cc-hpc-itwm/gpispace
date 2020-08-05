#pragma once

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

      template<typename T>
      class square_root_for_negative_argument : public std::runtime_error
      {
      public:
        square_root_for_negative_argument (T value)
          : std::runtime_error
            (( boost::format ("square root for negative argument '%1%'") % value
             ).str()
            )
          , _value (value)
        {}
        virtual ~square_root_for_negative_argument() = default;

      private:
        T _value;
      };

      template<typename T>
      class log_for_nonpositive_argument : public std::runtime_error
      {
      public:
        log_for_nonpositive_argument (T value)
          : std::runtime_error
            (( boost::format ("logarithm for nonpositive argument '%1%'")
             % value
             ).str()
            )
          , _value (value)
        {}
        virtual ~log_for_nonpositive_argument() = default;

      private:
        T _value;
      };
    }
  }
}
