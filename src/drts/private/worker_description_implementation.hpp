// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace gspc
{
  struct worker_description::worker_description_implementation
  {
    worker_description_implementation
      ( std::vector<std::string> capabilities_
      , std::size_t num_per_node_
      , std::size_t max_nodes_
      , std::size_t shm_size_
      , ::boost::optional<std::size_t> socket_
      , ::boost::optional<unsigned short> base_port_
      );
    worker_description_implementation (std::string);

    std::vector<std::string> capabilities;
    std::size_t num_per_node;
    std::size_t max_nodes;
    std::size_t shm_size;
    ::boost::optional<std::size_t> socket;
    ::boost::optional<unsigned short> base_port;
  };
}
