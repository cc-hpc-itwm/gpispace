// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <we/type/signature.hpp>
#include <we/type/value.hpp>

#include <string>

namespace we
{
  using workflow_response_callback
    = std::function<void ( pnet::type::value::value_type const& description
                         , pnet::type::value::value_type const& value
                         )>;

  std::string get_response_id (pnet::type::value::value_type const&);
  pnet::type::value::value_type make_response_description
    (std::string response_id, pnet::type::value::value_type const& value);

  bool is_response_description
    (pnet::type::signature::signature_type const&);

  static inline std::string response_description_requirements()
  {
    return "the field 'response_id :: string'";
  }
}
