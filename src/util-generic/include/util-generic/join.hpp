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

#include <util-generic/ostream/callback/function.hpp>

#include <boost/optional.hpp>

#include <algorithm>
#include <iterator>
#include <string>
#include <type_traits>

namespace fhg
{
  namespace util
  {
    template<typename C>
      using Container = typename std::remove_reference<C>::type;
    template<typename C>
      using Iterator = typename Container<C>::const_iterator;
    template<typename C>
      using Value = typename Container<C>::value_type;
    template<typename C>
      using Difference =
        typename std::iterator_traits<Iterator<C>>::difference_type;
    template<typename Iterator, typename Separator>
      class join_iterator;
    template<typename C, typename Separator>
      class join_reference;

    //! Create an ostream modifier that outputs the elements in \a c
    //! separated by \a s. The elements are printed using \a
    //! print. The sequence is prefixed by \a open and suffixed by \a
    //! close. If there are more than \a max_elements_to_print, the
    //! sequence is truncated once the limit is reached. If a limit is
    //! given, it is also indicated.
    //!
    //! Examples:
    //! - Given c = [1, 2, 3]; s = ", "; print = id; open = "["; close
    //!   = "]"; max_elements_to_print = none, the resulting output is
    //!   '[1, 2, 3]'.
    //! - Given c = [1, 2, 3]; s = "-"; print = id; open = "<"; close
    //!   = ">"; max_elements_to_print = 4, the resulting output is
    //!   '[3]: <1-2-3>'.
    //! - Given c = [1, 2, 3]; s = ", "; print = id; open = ""; close
    //!   = ""; max_elements_to_print = 1, the resulting output is
    //!   '[3]: 1...'.
    template<typename Container, typename Separator>
      constexpr join_reference<Container, Separator>
        join ( Container const& c
             , Separator s
             , ostream::callback::print_function<Value<Container>> const& print
             = ostream::callback::id<Value<Container>>()
             , std::string const& open = ""
             , std::string const& close = ""
             , boost::optional<Difference<Container>> const& max_elements_to_print
             = boost::none
             );

    //! Create an ostream modifier that outputs the elements between
    //! \a begin and \a end separated by \a separator. The elements
    //! are printed surrounded by \a local_open and \a local_close per
    //! element.
    //!
    //! Examples:
    //! - Given [begin, end) = [a, b, c]; separator = "-"; local_open
    //!   = ""; local_close = "", the resulting output is 'a-b-c'.
    //! - Given [begin, end) = [a, b]; separator = " "; local_open =
    //!   "{"; local_close = "}", the resulting output is '{a} {b}'.
    template<typename Iterator, typename Separator>
      constexpr join_iterator<Iterator, Separator>
        join ( Iterator begin, Iterator end
             , Separator separator
             , std::string const& local_open = ""
             , std::string const& local_close = ""
             );
  }
}

#include <util-generic/join.ipp>
