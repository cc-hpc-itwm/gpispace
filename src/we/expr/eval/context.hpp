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

#include <we/type/value.hpp>

#include <iosfwd>
#include <list>
#include <string>
#include <unordered_map>

namespace expr
{
  namespace eval
  {
    struct context
    {
    private:
      typedef std::unordered_map< std::string
                                  , pnet::type::value::value_type
                                  > container_type;

      container_type _container;

      typedef std::unordered_map< std::string
                                  , const pnet::type::value::value_type*
                                  > ref_container_type;

      ref_container_type _ref_container;

      friend std::ostream& operator<< (std::ostream&, context const&);

    public:
      void bind_ref (const std::string&, const pnet::type::value::value_type&);

      void bind_and_discard_ref ( const std::list<std::string>&
                                , const pnet::type::value::value_type&
                                );

      const pnet::type::value::value_type& value (const std::list<std::string>&) const;
    };
  }
}
