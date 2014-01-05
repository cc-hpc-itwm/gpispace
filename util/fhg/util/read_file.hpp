#ifndef FHG_UTIL_READ_FILE_HPP
#define FHG_UTIL_READ_FILE_HPP

#include <boost/filesystem.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    std::string read_file (boost::filesystem::path const&);
  }
}

#endif
