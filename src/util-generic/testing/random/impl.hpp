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

namespace fhg
{
  namespace util
  {
    namespace testing
    {
      namespace detail
      {
        //! \note Customization point. Overload or partially
        //! specialize for T, enabled by void_t or alike. Implement
        //! `T operator()() const`.
        //! \note See util-generic/testing/random/xxx.hpp for built-in
        //! specializations. These are automatically included, but
        //! moved there for readability.
        template<typename T, typename = void> struct random_impl {};

        //! Helper to specialize random_impl for <type_, void> and
        //! start defining the function.
        //! \note Must be used in global namespace.
#define FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE(type_)              \
        FHG_UTIL_TESTING_RANDOM_SPECIALIZE_SIMPLE_IMPL (type_)

        //! Helper to specialize random_impl for template<templ_>
        //! … <type_, cond_> and start defining the function.
        //! \note Must be used in global namespace.
#define FHG_UTIL_TESTING_RANDOM_SPECIALIZE_FULL(type_, cond_, templ_...) \
        FHG_UTIL_TESTING_RANDOM_SPECIALIZE_FULL_IMPL (type_, cond_, templ_)
      }
    }
  }
}

#include <util-generic/testing/random/impl.ipp>
