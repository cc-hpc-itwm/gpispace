// This file is part of GPI-Space.
// Copyright (C) 2022 Fraunhofer ITWM
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

#pragma once

#include <util-generic/callable_signature.hpp>

#include <boost/format.hpp>

#include <list>
#include <string>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      //! Require \a fun to throw an exception of type \c Exception,
      //! with the value \a expected. If an exception is indeed
      //! thrown, \a throw_if_mismatch is called with the \a expected
      //! and catched exception, which is then supposed to compare the
      //! two, throwing information about the mismatch if not equal.
      //! \note \a throw_if_mismatch is default constructed for
      //! convenience with stateless comparators, only having to
      //! specify exactly the comparator as template argument.
      template<typename ThrowIfMismatch, typename Exception, typename Fun>
        void require_exception
          ( Fun&& fun
          , Exception const& expected
          , ThrowIfMismatch throw_if_mismatch = ThrowIfMismatch()
          );

      //! Equivalent to \see require_exception(), defaulting \c
      //! ThrowIfMismatch to compare the exceptions by
      //! - \c operator==() if exists, else
      //! - string-comparing \c what()
      //! \note what() is a C string, implying that any messages
      //! containing zero bytes (e.g. from \c random_string()) will
      //! lead to incomplete comparison
      template<typename Exception, typename Fun>
        void require_exception ( Fun&& fun
                               , Exception const& expected
                               );

      namespace detail
      {
        template<typename Wrapping, typename Wrapped>
          struct nested_exception;
      }

      //! Create a wrapper around an exception \a wrapped, using \a
      //! wrapping as the outer exception, in order to hard-typedef
      //! nested exceptions for the \see require_exception() overloads
      //! below.
      //! \note Can wrap recursively to test hierarchies.
      template<typename Wrapping, typename Wrapped>
        detail::nested_exception<Wrapping, Wrapped> make_nested
          (Wrapping wrapping, Wrapped wrapped);

      //! Equivalent to \see require_exception(), but expecting a
      //! nested exception \a expected. \see make_nested() for
      //! creating such a nested expectance.
      template<typename ThrowIfMismatch, typename Wrapping, typename Wrapped, typename Fun>
        void require_exception
          ( Fun&& fun
          , detail::nested_exception<Wrapping, Wrapped> const& expected
          , ThrowIfMismatch throw_if_mismatch = ThrowIfMismatch()
          );
      //! Equivalent to \see require_exception(), but expecting a
      //! nested exception \a expected, and defaulting \c
      //! throw_if_mismatch to comparing by
      //! - \c operator==() if exists, else
      //! - string-comparing \c what()
      //! \note what() is a C string, implying that any messages
      //! containing zero bytes (e.g. from \c random_string()) will
      //! lead to incomplete comparison
      template<typename Wrapping, typename Wrapped, typename Fun>
        void require_exception
          ( Fun&& fun
          , detail::nested_exception<Wrapping, Wrapped> const& expected
          );

      //! Require an exception of type \a Exception to be thrown
      //! within \a fun, but instead of comparing the exception
      //! object, expect that the \c what() of the exception is one of
      //! the options given in \a whats.
      //! \note what() is a C string, implying that any messages
      //! containing zero bytes (e.g. from \c random_string()) will
      //! lead to incomplete comparison
      template<typename Exception, typename Fun>
        void require_exception_with_message_in
          ( Fun&& fun
          , std::list<std::string> const& whats
          );

      //! Equivalent to \see require_exception_with_message_in(), but
      //! taking \c whats in form of \a fmts, for convenience in
      //! construction.
      //! \note what() is a C string, implying that any messages
      //! containing zero bytes (e.g. from \c random_string()) will
      //! lead to incomplete comparison
      template<typename Exception, typename Fun>
        void require_exception_with_message_in
          ( Fun&& fun
          , std::list<::boost::format> const& fmts
          );

      //! Equivalent to \see require_exception_with_message_in(), but
      //! with just one option \a what.
      //! \note what() is a C string, implying that any messages
      //! containing zero bytes (e.g. from \c random_string()) will
      //! lead to incomplete comparison
      template<typename Exception, typename Fun>
        void require_exception_with_message ( Fun&& fun
                                            , std::string const& what
                                            );

      //! Equivalent to \see require_exception_with_message_in(), but
      //! with just one option for \c what, in form of \a fmt.
      //! \note what() is a C string, implying that any messages
      //! containing zero bytes (e.g. from \c random_string()) will
      //! lead to incomplete comparison
      template<typename Exception, typename Fun>
        void require_exception_with_message ( Fun&& fun
                                            , ::boost::format const& fmt
                                            );
    }
  }
}

#include <util-generic/testing/require_exception.ipp>
