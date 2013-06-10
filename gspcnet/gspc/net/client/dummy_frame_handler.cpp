#include "dummy_frame_handler.hpp"

#include <gspc/net/frame_handler.hpp>

#include <iostream>

namespace gspc
{
  namespace net
  {
    namespace
    {
      class dummy_frame_handler_t : public frame_handler_t
      {
        int handle_frame (user_ptr, frame const &)
        {
          return 0;
        }

        int handle_error (user_ptr, boost::system::error_code const &)
        {
          return 0;
        }
      };
    }

    frame_handler_t & dummy_frame_handler ()
    {
      static dummy_frame_handler_t dummy;
      return dummy;
    }
  }
}
