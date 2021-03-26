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

namespace fhg
{
  namespace util
  {
    namespace cxx17
    {
      template<typename...>
        struct conjunction : std::true_type
      {};
      template<typename B1>
        struct conjunction<B1> : B1
      {};
      template<typename B1, typename... Bn>
        struct conjunction<B1, Bn...>
          : std::conditional<!!B1::value, conjunction<Bn...>, B1>::type
      {};

      template<typename...>
        struct disjunction : std::false_type
      {};
      template<typename B1>
        struct disjunction<B1> : B1
      {};
      template<typename B1, typename... Bn>
        struct disjunction<B1, Bn...>
          : std::conditional<!!B1::value, B1, disjunction<Bn...>>::type
      {};

      template<typename B>
        struct negation : std::integral_constant<bool, !B::value>
      {};
    }
  }
}
