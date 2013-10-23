#include "echo.hpp"

#include <gspc/net/frame_builder.hpp>

namespace gspc
{
  namespace net
  {
    namespace service
    {
      void echo::operator() ( std::string const &
                            , frame const &rqst
                            , user_ptr user
                            )
      {
        frame rply = make::reply_frame (rqst);

        rply.set_body (rqst.get_body ());

        user->deliver (rply);
      }
    }
  }
}
