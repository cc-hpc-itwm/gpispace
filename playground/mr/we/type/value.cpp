// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      value_type empty()
      {
        return (std::map<std::string, value_type>());
      }
    }
  }
}
