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

#include <sdpa/capability.hpp>
#include <sdpa/id_generator.hpp>

namespace sdpa
{
  namespace
  {
    id_generator& GLOBAL_id_generator_cap()
    {
      static id_generator g ("cap");
      return g;
    }
  }

  Capability::Capability ( const std::string& name
                         , const std::string& owner
                         )
    : name_ (name)
    , depth_ (0)
    , owner_ (owner)
    , uuid_ (GLOBAL_id_generator_cap().next())
    {}
}
