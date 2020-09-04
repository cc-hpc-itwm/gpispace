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
#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace gspc
{
  struct worker_description
  {
    std::vector<std::string> capabilities;
    std::size_t num_per_node;
    std::size_t max_nodes;
    std::size_t shm_size;
    boost::optional<std::size_t> socket;
    boost::optional<unsigned short> base_port;
  };
}
