#ifndef FHG_COM_BASIC_SESSION_HPP
#define FHG_COM_BASIC_SESSION_HPP 1

#include <string>

namespace fhg
{
  namespace com
  {
    class basic_session
    {
    public:
      virtual ~basic_session () {}

      virtual std::string local_endpoint_str () const = 0;
      virtual std::string remote_endpoint_str () const = 0;
      virtual void async_send (const std::string &) = 0;
    };
  }
}

#endif
