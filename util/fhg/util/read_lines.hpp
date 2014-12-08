#ifndef FHG_UTIL_READ_LINES_HPP
#define FHG_UTIL_READ_LINES_HPP

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

#endif
