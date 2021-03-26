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
    namespace mp
    {
      namespace detail
      {
        template<template<typename> class, typename> struct apply;
      }

      //! Apply an `Operation<typename> op` to all elements of
      //! type-sequence `Sequence = sequence_type<Types...>` and
      //! return `sequence_type<op (Types)...>`.
      template<template<typename> class Operation, typename Sequence>
        using apply = typename detail::apply<Operation, Sequence>::type;
    }
  }
}

#include <util-generic/mp/apply.ipp>
