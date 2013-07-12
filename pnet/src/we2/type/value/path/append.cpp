// mirko.rahn@itwm.fraunhofer.de

#include <we2/type/value/path/append.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      namespace path
      {
        append::append ( std::list<std::string>& path
                       , const std::string& key
                       )
          : _path (path)
        {
          _path.push_back (key);
        }
        append::~append()
        {
          _path.pop_back();
        }
        append::operator std::list<std::string>&() const
        {
          return _path;
        }
      }
    }
  }
}
