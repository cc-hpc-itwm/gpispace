#pragma once

#include <boost/optional.hpp>

#include <vector>
#include <string>

namespace we
{
  namespace test
  {
    namespace buffer_alignment
    {
struct BufferInfo
{
  std::string name;
  unsigned long size;
  boost::optional<unsigned long> alignment;
};

std::string create_net_description (std::vector<BufferInfo> const&);
    }
  }
}
