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

#include <drts/information_to_reattach.hpp>
#include <drts/private/host_and_port.hpp>

#include <string>

namespace gspc
{
  struct information_to_reattach::implementation
  {
    explicit
    implementation (host_and_port_type const&);

    explicit
    implementation (std::string const&);

    std::string to_string() const;

    host_and_port_type const& endpoint() const;
  private:
    host_and_port_type _endpoint;
  };
}
