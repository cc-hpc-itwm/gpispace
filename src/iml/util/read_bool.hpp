#pragma once

#include <string>

namespace fhg
{
  namespace iml
  {
    namespace util
    {
      //! \note Same as fhg::iml::util::parse::require::boolean, but with ::tolower()
      bool read_bool (const std::string&);
    }
  }
}
