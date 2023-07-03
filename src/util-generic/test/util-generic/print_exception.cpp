// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <util-generic/print_exception.hpp>
#include <util-generic/testing/random.hpp>

#include <boost/test/unit_test.hpp>

namespace fhg
{
  namespace util
  {
    namespace
    {
      template<typename T>
        std::exception_ptr mk (T&& x)
      {
        return std::make_exception_ptr (std::forward<T> (x));
      }
      template<typename T>
        std::exception_ptr mk (T&& x, std::exception_ptr inner)
      {
        try
        {
          try
          {
            std::rethrow_exception (inner);
          }
          catch (...)
          {
            std::throw_with_nested (std::forward<T> (x));
          }
        }
        catch (...)
        {
          return std::current_exception();
        }
      }

      std::string random_cstr_comparable()
      {
        return testing::random<std::string>{}
          (testing::random<char>::any_without_zero());
      }
    }

    BOOST_AUTO_TEST_CASE (unknown_exception_for_non_std_exceptions)
    {
      BOOST_REQUIRE_EQUAL
        ( exception_printer (mk (1)).string()
        , "unknown exception type"
        );
    }

    namespace
    {
      struct user_defined_exception : std::exception
      {
        char const* _message;
        user_defined_exception (std::string const& message)
          : _message (message.c_str())
        {}
        virtual char const* what() const noexcept override { return _message; }
      };
    }

    BOOST_AUTO_TEST_CASE (std_exception_types_just_print_what)
    {
      auto const message (random_cstr_comparable());
      BOOST_REQUIRE_EQUAL
        ( exception_printer (mk (std::runtime_error (message))).string()
        , message
        );

      BOOST_REQUIRE_EQUAL
        ( exception_printer (mk (std::logic_error (message))).string()
        , message
        );

      BOOST_REQUIRE_EQUAL
        ( exception_printer (mk (user_defined_exception (message))).string()
        , message
        );
    }

    BOOST_AUTO_TEST_CASE (nested_exceptions_are_printed)
    {
      auto const inner (random_cstr_comparable());
      auto const mid (random_cstr_comparable());
      auto const outer (random_cstr_comparable());

      BOOST_REQUIRE_EQUAL
        ( exception_printer ( mk ( std::logic_error (outer)
                                 , mk ( std::runtime_error (mid)
                                      , mk (std::logic_error (inner))
                                      )
                                 )
                            ).string()
        , outer + "\n " + mid + "\n  " + inner
        );
    }

    BOOST_AUTO_TEST_CASE (nested_exceptions_are_printed_with_given_indentation)
    {
      auto const inner (random_cstr_comparable());
      auto const mid (random_cstr_comparable());
      auto const outer (random_cstr_comparable());

      BOOST_REQUIRE_EQUAL
        ( exception_printer ( mk ( std::logic_error (outer)
                                 , mk ( std::runtime_error (mid)
                                      , mk (std::logic_error (inner))
                                      )
                                 )
                            , 3
                            ).string()
        , "   " + outer + "\n    " + mid + "\n     " + inner
        );
    }

    BOOST_AUTO_TEST_CASE (nested_exceptions_are_printed_with_given_separator)
    {
      auto const inner (random_cstr_comparable());
      auto const mid (random_cstr_comparable());
      auto const outer (random_cstr_comparable());
      auto const separator (random_cstr_comparable());

      BOOST_REQUIRE_EQUAL
        ( exception_printer ( mk ( std::logic_error (outer)
                                 , mk ( std::runtime_error (mid)
                                      , mk (std::logic_error (inner))
                                      )
                                 )
                            , separator
                            ).string()
        , outer + separator + mid + separator + inner
        );
    }

    BOOST_AUTO_TEST_CASE (current_exception_overload)
    {
      auto const message (random_cstr_comparable());

      try
      {
        throw std::runtime_error (message);
      }
      catch (...)
      {
        BOOST_REQUIRE_EQUAL (current_exception_printer().string(), message);
        return;
      }
    }
  }
}
