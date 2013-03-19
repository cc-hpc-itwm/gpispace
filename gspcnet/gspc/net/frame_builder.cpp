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
        frame f;
        f.set_command ("ERROR");
        f.set_header ("content-type", "text/plain");
        f.set_header ("code", boost::lexical_cast<std::string>(ec));
        f.set_header ("message", message);
        return f;
      }
    }
  }
}
