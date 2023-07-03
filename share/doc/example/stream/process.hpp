// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

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
