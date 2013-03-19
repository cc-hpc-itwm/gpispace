#include "echo.hpp"

#include <gspc/net/frame.hpp>

namespace gspc
{
  namespace net
  {
    namespace service
    {
      void echo::operator() ( std::string const &
                            , frame const &rqst
                            , frame & rply
                            )
      {
        rply = rqst;
      }
    }
  }
}
