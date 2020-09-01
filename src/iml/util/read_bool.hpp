#pragma once

#include <string>

namespace fhg
{
  namespace util
  {
    //! \note Same as fhg::util::parse::require::boolean, but with ::tolower()
    bool read_bool (const std::string&);
  }
}
