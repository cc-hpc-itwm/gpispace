// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/test/unit_test.hpp>

#include <fmt/core.h>
#include <util-generic/testing/random.hpp>
#include <util-generic/testing/require_exception.hpp>

namespace fhg
{
  namespace util
  {
    namespace testing
    {

#define SUCCESS(_name, ...)                                   \
      BOOST_AUTO_TEST_CASE (_name ## _success)                \
      {                                                       \
        __VA_ARGS__;                                          \
      }

#define FAILURE(_name, ...)                                   \
      BOOST_AUTO_TEST_CASE (_name ## _failure)                \
      {                                                       \
        BOOST_REQUIRE_THROW (__VA_ARGS__, std::logic_error);  \
      }

      SUCCESS ( int_default_comp
              , require_exception
                  ( [] { throw 1; }
                  , 1
                  )
              )
      FAILURE ( int_default_comp_value_mismatch
              , require_exception
                  ( [] { throw 2; }
                  , 1
                  )
              )
      FAILURE ( int_default_comp_missing
              , require_exception
                  ( [] {}
                  , 1
                  )
              )


      namespace
      {
        void require_int_equal (int lhs, int rhs)
        {
          if (lhs != rhs)
          {
            throw std::logic_error ("");
          }
        }
      }

      SUCCESS ( int_specific_comp
              , require_exception
                  ( [] { throw 1; }
                  , 1
                  , require_int_equal
                  )
              )
      FAILURE ( int_specific_comp
              , require_exception
                  ( [] { throw 2; }
                  , 1
                  , require_int_equal
                  )
              )


      SUCCESS ( runtime_error_default_comp
              , require_exception
                  ( [] { throw std::runtime_error ("foo"); }
                  , std::runtime_error ("foo")
                  )
              )
      FAILURE ( runtime_error_default_comp
              , require_exception
                  ( [] { throw std::runtime_error ("bar"); }
                  , std::runtime_error ("foo")
                  )
              )


      namespace
      {
        void require_runtime_error_what
          (std::runtime_error const& lhs, std::runtime_error const& rhs)
        {
          if (std::string (lhs.what()) != rhs.what())
          {
            throw std::logic_error ("");
          }
        }
      }

      SUCCESS ( runtime_error_specific_comp
              , require_exception
                  ( [] { throw std::runtime_error ("foo"); }
                  , std::runtime_error ("foo")
                  , &require_runtime_error_what
                  )
              )
      FAILURE ( runtime_error_specific_comp
              , require_exception
                  ( [] { throw std::runtime_error ("bar"); }
                  , std::runtime_error ("foo")
                  , &require_runtime_error_what
                  )
              )


      SUCCESS ( nested_default_comp
              , require_exception
                  ( []
                    {
                      try
                      {
                        throw std::runtime_error ("bar");
                      }
                      catch (...)
                      {
                        std::throw_with_nested (std::runtime_error ("foo"));
                      }
                    }
                  , make_nested ( std::runtime_error ("foo")
                                , std::runtime_error ("bar")
                                )
                  )
              )
      FAILURE ( nested_default_comp_one_content_wrong
              , require_exception
                  ( []
                    {
                      try
                      {
                        throw std::runtime_error ("foo");
                      }
                      catch (...)
                      {
                        std::throw_with_nested (std::runtime_error ("bar"));
                      }
                    }
                  , make_nested ( std::runtime_error ("foo")
                                , std::runtime_error ("bar")
                                )
                  )
              )
      FAILURE ( nested_default_comp_one_type_wrong
              , require_exception
                  ( []
                    {
                      try
                      {
                        throw std::runtime_error ("bar");
                      }
                      catch (...)
                      {
                        std::throw_with_nested (std::logic_error ("foo"));
                      }
                    }
                  , make_nested ( std::runtime_error ("foo")
                                , std::runtime_error ("bar")
                                )
                  )
              )
      FAILURE ( nested_default_comp_not_even_nested
              , require_exception
                  ( []
                    {
                    throw 1;
                    }
                  , make_nested ( std::runtime_error ("foo")
                                , std::runtime_error ("bar")
                                )
                  )
              )

      SUCCESS ( by_message
              , require_exception_with_message<std::logic_error>
                  ( [] { throw std::logic_error ("foo"); }
                  , "foo"
                  )
              )
      FAILURE ( by_message
              , require_exception_with_message<std::logic_error>
                  ( [] { throw std::logic_error ("bar"); }
                  , "foo"
                  )
              )
      FAILURE ( by_message_type_mismatch
              , require_exception_with_message<std::runtime_error>
                  ( [] { throw std::logic_error ("bar"); }
                  , "foo"
                  )
              )

      SUCCESS ( by_message_fmt
              , require_exception_with_message<std::logic_error>
                  ( [] { throw std::logic_error ("foo"); }
                  , fmt::format ("{}", "foo")
                  )
              )
      FAILURE ( by_message_fmt
              , require_exception_with_message<std::logic_error>
                  ( [] { throw std::logic_error ("bar"); }
                  , fmt::format ("{}", "foo")
                  )
              )

      SUCCESS ( by_message_in
              , require_exception_with_message_in<std::logic_error>
                  ( []
                    {
                      throw std::logic_error
                        (std::string (1, random<char>{} ("fb")));
                    }
                    , {std::string ("f"), std::string ("b")}
                  )
              )
      FAILURE ( by_message_in
              , require_exception_with_message_in<std::logic_error>
                  ( [] { throw std::logic_error ("baz"); }
                  , {std::string ("foo"), std::string ("bar")}
                  )
              )
      FAILURE ( by_message_in_type_mismatch
              , require_exception_with_message_in<std::logic_error>
                  ( [] { throw "baz"; }
                  , {std::string ("foo"), std::string ("bar")}
                  )
              )

      FAILURE ( by_message_in_fmt
              , require_exception_with_message_in<std::logic_error>
                  ( [] { throw std::logic_error ("baz"); }
                  , { fmt::format ("{}", "foo")
                    , fmt::format ("{}", "bar")
                    }
                  )
              )

      namespace
      {
        std::string random_cstr_comparable()
        {
          return random<std::string>{} (random<char>::any_without_zero());
        }
      }

      BOOST_AUTO_TEST_CASE (message_got_no_exception_at_all)
      {
        std::logic_error const expected (random_cstr_comparable());

        require_exception
          ( [&] { require_exception ([]{}, expected); }
          , std::logic_error
              { fmt::format ( "missing exception '{}' of type {}: {}"
                            , expected.what()
                            , typeid (expected).name()
                            , "got no exception at all"
                            )
              }
          );
      }

      BOOST_AUTO_TEST_CASE (message_got_exception_of_wrong_type_unknown)
      {
        std::logic_error const expected (random_cstr_comparable());
        auto const actual (random<std::string>{}());

        require_exception
          ( [&] { require_exception ([&] { throw actual; }, expected); }
          , std::logic_error
              { fmt::format ( "missing exception '{}' of type {}: {}"
                            , expected.what()
                            , typeid (expected).name()
                            , "got exception of wrong type: unknown exception type"
                            )
              }
          );
      }

      BOOST_AUTO_TEST_CASE (message_got_exception_of_wrong_type)
      {
        std::logic_error const expected (random_cstr_comparable());
        std::runtime_error const actual (random_cstr_comparable());

        require_exception
          ( [&] { require_exception ([&] { throw actual; }, expected); }
          , std::logic_error
              { fmt::format
                  ( "missing exception '{}' of type {}: {}{}: {}"
                  , expected.what()
                  , typeid (expected).name()
                  , "got exception of wrong type "
                  , typeid (actual).name()
                  , actual.what()
                  )
              }
          );
      }

      BOOST_AUTO_TEST_CASE (message_missing_nested_exception)
      {
        std::runtime_error const expected_nesting (random_cstr_comparable());
        std::logic_error const expected_nested (random_cstr_comparable());
        auto const expected (make_nested (expected_nesting, expected_nested));

        require_exception
          ( [&]
            {
              require_exception
                ( [&] { throw expected_nesting; }
                , expected
                );
            }
          , std::logic_error
              { fmt::format ( "missing exception '{0}: {1}' of type {2}: missing nested exception of type {3}"
                            , expected_nesting.what()
                            , expected_nested.what()
                            , typeid (expected).name()
                            , typeid (expected_nested).name()
                            )
              }
          );
      }

      BOOST_AUTO_TEST_CASE (message_nested_exception_of_wrong_type)
      {
        std::runtime_error const expected_nesting (random_cstr_comparable());
        std::logic_error const expected_nested (random_cstr_comparable());
        std::runtime_error const actual_nested (random_cstr_comparable());
        auto const expected (make_nested (expected_nesting, expected_nested));

        require_exception
          ( [&]
            {
              require_exception
                ( [&]
                  {
                    try { throw actual_nested; }
                    catch (...) { std::throw_with_nested (expected_nesting); }
                  }
                , expected
                );
            }
          , std::logic_error
              { fmt::format ( "missing exception '{0}: {1}' of type {2}: nested exception of wrong type: got {3}, expected {4}"
                            , expected_nesting.what()
                            , expected_nested.what()
                            , typeid (expected).name()
                            , typeid (actual_nested).name()
                            , typeid (expected_nested).name()
                            )
              }
          );
      }

      BOOST_AUTO_TEST_CASE (message_nested_exception_of_wrong_type_unknown)
      {
        std::runtime_error const expected_nesting (random_cstr_comparable());
        std::logic_error const expected_nested (random_cstr_comparable());
        std::string const actual_nested (random_cstr_comparable());
        auto const expected (make_nested (expected_nesting, expected_nested));

        require_exception
          ( [&]
            {
              require_exception
                ( [&]
                  {
                    try { throw actual_nested; }
                    catch (...) { std::throw_with_nested (expected_nesting); }
                  }
                , expected
                );
            }
          , std::logic_error
              { fmt::format ( "missing exception '{0}: {1}' of type {2}: nested exception of wrong type: got unknown, expected {3}"
                            , expected_nesting.what()
                            , expected_nested.what()
                            , typeid (expected).name()
                            , typeid (expected_nested).name()
                            )
              }
          );
      }
    }
  }
}
