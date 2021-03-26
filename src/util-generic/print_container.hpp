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

#include <util-generic/join.hpp>

namespace fhg
{
  namespace util
  {
    template<typename C>
      constexpr join_reference<C, std::string>
        print_container
          ( const std::string& open
          , const std::string& separator
          , const std::string& close
          , C const& c
          , ostream::callback::print_function<Value<C>> const& print
          = ostream::callback::id<Value<C>>()
          , boost::optional<typename std::iterator_traits<typename std::remove_reference<C>::type::const_iterator>::difference_type>
              const& max_elements_to_print = boost::none
          )
    {
      return join_reference<C, std::string>
        (c, separator, print, open, close, max_elements_to_print);
    }
  }
}
