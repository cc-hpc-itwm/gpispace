#pragma once

#include <boost/optional.hpp>

#include <vector>
#include <string>

struct BufferInfo
{
  std::string name;
  unsigned long size;
  boost::optional<unsigned long> alignment;

  BufferInfo (std::string, unsigned long, boost::optional<unsigned long>);
};

std::string create_net_description (std::vector<BufferInfo> const&);
