#pragma once

#include <boost/filesystem.hpp>

#include <string>

namespace fhg
{
  namespace util
  {
    std::string read_file (boost::filesystem::path const&);
  }
}
