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

#include <we/type/literal/control.hpp>
#include <we/type/bitsetofint.hpp>
#include <we/type/bytearray.hpp>

#include <boost/variant.hpp>

#include <list>
#include <map>
#include <set>
#include <string>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      typedef boost::make_recursive_variant
              < we::type::literal::control
              , bool
              , int
              , long
              , unsigned int
              , unsigned long
              , float
              , double
              , char
              , std::string
              , bitsetofint::type
              , we::type::bytearray
              , std::list<boost::recursive_variant_>
              , std::set<boost::recursive_variant_>
              , std::map<boost::recursive_variant_, boost::recursive_variant_>
              , std::list<std::pair<std::string, boost::recursive_variant_> >
              >::type value_type;

      typedef std::list<std::pair<std::string, value_type> > structured_type;
    }
  }
}
