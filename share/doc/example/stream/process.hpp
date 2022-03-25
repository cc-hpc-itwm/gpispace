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

#include <statistic.hpp>

#include <sstream>
#include <string>

namespace share_example_stream
{
  std::pair<unsigned long, std::chrono::high_resolution_clock::rep>
    process (std::pair<void const*, unsigned long> ptr_data)
  {
    static fhg::util::statistic delta ("process: delta");
    static fhg::util::statistic duration ("process: duration");

    std::chrono::high_resolution_clock::rep const start (delta.now());

    char const* const data (static_cast<char const*> (ptr_data.first));

    std::istringstream iss (std::string (data, data + ptr_data.second));

    unsigned long id;
    iss >> id;

    std::chrono::high_resolution_clock::rep produced;
    iss >> produced;

    delta.tick (start - produced);

    duration.tick (duration.now() - start);

    return {id, produced};
  }
}
