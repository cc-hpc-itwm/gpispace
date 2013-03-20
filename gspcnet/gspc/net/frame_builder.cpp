#include "frame_builder.hpp"

#include <boost/lexical_cast.hpp>

namespace gspc
{
  namespace net
  {
    namespace make
    {
      frame error_frame ( int ec
                        , const char *message
                        )
      {
        frame f ("ERROR");
        f.set_header ("content-type", "text/plain");
        f.set_header ("code", boost::lexical_cast<std::string>(ec));
        f.set_header ("message", message);
        return f;
      }

      frame receipt_frame (frame::value_type const &id)
      {
        frame f ("RECEIPT");
        f.set_header ("receipt-id", id);
        return f;
      }
    }
  }
}
