#pragma once

#include <boost/filesystem/path.hpp>

#include <string>
#include <vector>

namespace fhg
{
  namespace util
  {
    std::vector<std::string> read_lines (boost::filesystem::path const&);
  }
}
