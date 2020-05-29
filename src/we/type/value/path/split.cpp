#include <we/type/value/path/split.hpp>

#include <util-generic/split.hpp>

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
