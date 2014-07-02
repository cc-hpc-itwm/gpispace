#include <unistd.h>

#include "environment.hpp"
#include "split.hpp"

namespace fhg
{
  namespace util
  {
    std::map<std::string, std::string> environment ()
    {
      std::map<std::string, std::string> env_map;

      for (char **entry = environ ; entry && *entry != nullptr ; ++entry)
      {
        env_map.insert (fhg::util::split_string (*entry, '='));
      }

      return env_map;
    }
  }
}
