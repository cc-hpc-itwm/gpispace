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

#include <xml/parse/state.hpp>

#include <we/type/property.hpp>

namespace xml
{
  namespace parse
  {
    namespace util
    {
      namespace property
      {
        void set_state ( state::type& state
                       , we::type::property::type& prop
                       , we::type::property::path_type const& path
                       , we::type::property::value_type const& value
                       );
        void join ( state::type const& state
                  , we::type::property::type& x
                  , we::type::property::type const& y
                  );
      }
    }
  }
}
