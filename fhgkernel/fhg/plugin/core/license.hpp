#ifndef FHG_PLUGIN_CORE_LICENSE_HPP
#define FHG_PLUGIN_CORE_LICENSE_HPP

#include <string>

namespace fhg
{
  namespace plugin
  {
    enum license_errors
      {
        LICENSE_VALID = 0
      , LICENSE_NOT_VERIFYABLE
      , LICENSE_CORRUPT
      , LICENSE_EXPIRED
      , LICENSE_INVALID
      , LICENSE_VERSION_MISMATCH
      };

    int check_license (std::string const &license);
    int check_license_file (std::string const &path);
  }
}

#endif
