// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#include <boost/test/unit_test.hpp>

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
                  , boost::format ("%1%") % "foo"
                  )
              )
      FAILURE ( by_message_fmt
              , require_exception_with_message<std::logic_error>
                  ( [] { throw std::logic_error ("bar"); }
                  , boost::format ("%1%") % "foo"
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
                  , { boost::format ("%1%") % "foo"
                    , boost::format ("%1%") % "bar"
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
              ( ( boost::format ("missing exception '%1%' of type %2%: %3%")
                % expected.what()
                % typeid (expected).name()
                % "got no exception at all"
                ).str()
              )
          );
      }

      BOOST_AUTO_TEST_CASE (message_got_exception_of_wrong_type_unknown)
      {
        std::logic_error const expected (random_cstr_comparable());
        auto const actual (random<std::string>{}());

        require_exception
          ( [&] { require_exception ([&] { throw actual; }, expected); }
          , std::logic_error
              ( ( boost::format ("missing exception '%1%' of type %2%: %3%")
                % expected.what()
                % typeid (expected).name()
                % "got exception of wrong type: unknown exception type"
                ).str()
              )
          );
      }

      BOOST_AUTO_TEST_CASE (message_got_exception_of_wrong_type)
      {
        std::logic_error const expected (random_cstr_comparable());
        std::runtime_error const actual (random_cstr_comparable());

        require_exception
          ( [&] { require_exception ([&] { throw actual; }, expected); }
          , std::logic_error
              ( ( boost::format
                    ("missing exception '%1%' of type %2%: %3%%4%: %5%")
                % expected.what()
                % typeid (expected).name()
                % "got exception of wrong type "
                % typeid (actual).name()
                % actual.what()
                ).str()
              )
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
              ( ( boost::format ("missing exception '%1%: %2%' of type %3%: missing nested exception of type %4%")
                % expected_nesting.what()
                % expected_nested.what()
                % typeid (expected).name()
                % typeid (expected_nested).name()
                ).str()
              )
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
              ( ( boost::format ("missing exception '%1%: %2%' of type %3%: nested exception of wrong type: got %4%, expected %5%")
                % expected_nesting.what()
                % expected_nested.what()
                % typeid (expected).name()
                % typeid (actual_nested).name()
                % typeid (expected_nested).name()
                ).str()
              )
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
              ( ( boost::format ("missing exception '%1%: %2%' of type %3%: nested exception of wrong type: got unknown, expected %4%")
                % expected_nesting.what()
                % expected_nested.what()
                % typeid (expected).name()
                % typeid (expected_nested).name()
                ).str()
              )
          );
      }
    }
  }
}
