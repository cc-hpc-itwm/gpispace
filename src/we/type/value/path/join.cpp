// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/path/join.hpp>

#include <util-generic/join.hpp>

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
          return fhg::util::join (path, '.');
        }
      }
    }
  }
}
