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

#pragma once

namespace fhg
{
  namespace util
  {
    namespace detail
    {
      template<typename> struct callable_signature_t;
      template<typename> struct return_type_t;
    }

    //! Get signature of callable Callable t, i.e. t() or
    //! Callable::operator().
    template<typename Callable>
      using callable_signature
        = typename detail::callable_signature_t<Callable>::type;

    //! Test if Callable can be invoked with WithSignature, including
    //! implicit conversions of arguments.
    //! \note included in c++17 N4446/P0077R0: "is_callable, the
    //! missing INVOKE related trait"
    template<typename Callable, typename WithSignature>
      struct is_callable;

    //! Get the return type of a Callable.
    template<typename Callable>
      using return_type = typename detail::return_type_t<Callable>::type;
  }
}

#include <util-generic/callable_signature.ipp>
