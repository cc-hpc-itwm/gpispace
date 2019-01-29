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
  };
}
