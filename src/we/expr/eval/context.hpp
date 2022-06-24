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

#include <gspc/detail/dllexport.hpp>

#include <we/type/value.hpp>

#include <iosfwd>
#include <list>
#include <string>
#include <unordered_map>

namespace expr
{
  namespace eval
  {
    struct GSPC_DLLEXPORT context
    {
    private:
      std::unordered_map<std::string, pnet::type::value::value_type>
        _container;
      std::unordered_map<std::string, const pnet::type::value::value_type*>
        _ref_container;

      GSPC_DLLEXPORT
        friend std::ostream& operator<< (std::ostream&, context const&);

    public:
      void bind_ref (std::string const&, pnet::type::value::value_type const&);

      void bind_and_discard_ref ( std::list<std::string> const&
                                , pnet::type::value::value_type const&
                                );

      pnet::type::value::value_type const& value (std::list<std::string> const&) const;
    };
  }
}
