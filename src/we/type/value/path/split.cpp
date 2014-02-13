// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/path/split.hpp>

#include <fhg/util/split.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        std::list<std::string> split (const std::string& path)
        {
          return fhg::util::split<std::string, std::string> (path, '.');
        }
      }
    }
  }
}
