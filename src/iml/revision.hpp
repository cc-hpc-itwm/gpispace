#pragma once

#include <string>

namespace fhg
{
  extern const char* project_version();
  extern const char* project_revision();

  extern std::string project_info (const std::string&);
}
