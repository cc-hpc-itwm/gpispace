// mirko.rahn@itwm.fraunhofer.de

#include <we/type/value/option.hpp>

namespace pnet
{
  namespace type
  {
    namespace value
    {
      bool& show_signatures_full()
      {
        static bool x (true);

        return x;
      }
    }
  }
}
