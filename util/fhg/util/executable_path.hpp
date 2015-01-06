// bernd.loerwald@itwm.fraunhofer.de

#ifndef FHG_UTIL_EXECUTABLE_PATH_HPP
#define FHG_UTIL_EXECUTABLE_PATH_HPP

#include <boost/filesystem/path.hpp>

namespace fhg
{
  namespace util
  {
    boost::filesystem::path executable_path();
  }
}

#endif
