// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
          , ::boost::optional<typename std::iterator_traits<typename std::remove_reference<C>::type::const_iterator>::difference_type>
              const& max_elements_to_print = ::boost::none
          )
    {
      return join_reference<C, std::string>
        (c, separator, print, open, close, max_elements_to_print);
    }
  }
}
