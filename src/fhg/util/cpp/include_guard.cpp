// mirko.rahn@itwm.fraunhofer.de

#include <fhg/util/cpp/include_guard.hpp>

#include <iostream>

namespace fhg
{
  namespace util
  {
    namespace cpp
    {
      namespace include_guard
      {
        open::open (const std::string& name)
          : _name (name)
        {}
        std::ostream& open::operator() (std::ostream& os) const
        {
          return os << "#ifndef _" << _name << std::endl
                    << "#define _" << _name << std::endl
                    << std::endl;
        }

        std::ostream& close::operator() (std::ostream& os) const
        {
          return os << "#endif" << std::endl;
        }
      }
    }
  }
}
