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

#include <we/field.hpp>
#include <we/type/value/peek.hpp>

namespace pnet
{
  type::value::value_type const& field
    ( std::string const& f
    , type::value::value_type const& v
    , type::signature::signature_type const& signature
    )
  {
    boost::optional<type::value::value_type const&> field
      (type::value::peek (f, v));

    if (!field)
    {
      throw exception::missing_field ( signature
                                     , v
                                     , std::list<std::string> (1, f)
                                     );
    }

    return *field;
  }
}
