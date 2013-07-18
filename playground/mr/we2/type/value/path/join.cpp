// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/value/path/join.hpp>

#include <fhg/util/join.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        std::string join (const std::list<std::string>& path)
        {
          return fhg::util::join (path, ".");
        }
      }
    }
  }
}
