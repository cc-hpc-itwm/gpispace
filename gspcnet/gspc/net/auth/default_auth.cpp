#include "default_auth.hpp"
#include "simple.hpp"

namespace gspc
{
  namespace net
  {
    namespace auth
    {
      auth_t & default_auth ()
      {
        static simple_auth_t a;
        return a;
      }
    }
  }
}
