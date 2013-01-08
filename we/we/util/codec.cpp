// rahn@itwm.fhg.de

#include <we/util/codec.hpp>

//! \todo remove
#include <we/type/net.hpp>

namespace we
{
  namespace util
  {
    namespace codec
    {
      we::mgmt::type::activity_t decode (std::istream& s)
      {
        we::mgmt::type::activity_t t;
        decode (s, t);
        return t;
      }

      we::mgmt::type::activity_t decode (const std::string& s)
      {
        std::istringstream iss (s);

        return decode (iss);
      }
    };
  }
}
