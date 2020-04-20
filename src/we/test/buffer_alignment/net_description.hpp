#pragma once

#include <boost/optional.hpp>

#include <vector>
#include <string>

struct BufferInfo
{
  std::string name;
  unsigned long size;
  boost::optional<unsigned long> alignment;
};

std::string create_net_description (std::vector<BufferInfo> const&);
