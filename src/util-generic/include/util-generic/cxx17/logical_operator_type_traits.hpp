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

//! \note P0013R1: Logical Operator Type Traits (revision 1)

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename... B> struct conjunction;
      template<typename... B> struct disjunction;
      template<typename B> struct negation;
    }
  }
}

#include <util-generic/cxx17/logical_operator_type_traits.ipp>
