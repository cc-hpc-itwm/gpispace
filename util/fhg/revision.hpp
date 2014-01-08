// alexander.petry@itwm.fraunhofer.de

#ifndef FHG_UTIL_REVISION_HPP
#define FHG_UTIL_REVISION_HPP 1

#include <string>

namespace fhg
{
  extern const char* project_summary();
  extern const char* project_version();
  extern const char* project_revision();

  extern std::string project_info (const std::string&);
}

#endif
