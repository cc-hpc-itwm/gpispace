// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
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

#include <util-generic/testing/printer/generic.hpp>
#include <util-generic/warning.hpp>

#include <iostream>
#include <tuple>

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace printer
      {
        namespace
        {
          template<bool = true> struct maybe_comma
          {
            static void apply (std::ostream& os) { os << ", "; }
          };
          template<> struct maybe_comma<false>
          {
            //! \note false positive in clang 3.8.0 (if maybe_comma<false>
            //! is unused, maybe_comma<true> would have to be unused
            //! as well as that can only happen in the case of empty
            //! tuples in which case maybe_comma is entirely
            //! unused. The inverse, only no-comma being used, would
            //! be possible with one-entry-tuples. Note that removing
            //! this function also lets compilation fail.)
            DISABLE_WARNING_CLANG ("-Wunused-member-function")
            static void apply (std::ostream&) {}
            RESTORE_WARNING_CLANG ("-Wunused-member-function")
          };

          template<typename tuple, std::size_t remaining_size>
            struct printer_impl
          {
            static void apply (std::ostream& os, tuple const& t)
            {
              constexpr std::size_t const next (remaining_size - 1);
              printer_impl<tuple, next>::apply (os, t);
              maybe_comma<next != 0>::apply (os);
              os << FHG_BOOST_TEST_PRINT_LOG_VALUE_HELPER (std::get<next> (t));
            }
          };
          template<typename tuple>
            struct printer_impl<tuple, 0>
          {
            static void apply (std::ostream&, tuple const&) {}
          };
        }
      }
    }
  }
}

FHG_BOOST_TEST_TEMPLATED_LOG_VALUE_PRINTER
  (<typename... Args>, std::tuple<Args...>, os, t)
{
  os << "<";
  using fhg::util::testing::printer::printer_impl;
  printer_impl<std::tuple<Args...>, sizeof... (Args)>::apply (os, t);
  os << ">";
}
