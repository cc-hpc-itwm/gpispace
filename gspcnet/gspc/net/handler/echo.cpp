#include "echo.hpp"

#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    namespace handler
    {
      int echo::operator() ( std::string const &
                           , frame const &rqst
                           , frame & rply
                           )
      {
        rply = rqst;
        return 0;
      }
    }
  }
}
