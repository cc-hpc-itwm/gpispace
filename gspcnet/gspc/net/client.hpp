#ifndef GSPC_NET_CLIENT_HPP
#define GSPC_NET_CLIENT_HPP

#include <string>
#include <gspc/net/frame_fwd.hpp>
#include <gspc/net/frame_handler_fwd.hpp>

namespace gspc
{
  namespace net
  {
    class client_t
    {
    public:
      virtual ~client_t () {}

      virtual int start () = 0;
      virtual int stop () = 0;

      virtual void set_frame_handler (frame_handler_t &) = 0;

      virtual int send_raw (frame const &) = 0;

      virtual int request (frame const &rqst, frame &rply) = 0;
      virtual int send (frame const &) = 0;
      virtual int subscribe ( std::string const &dest
                            , std::string const &id
                            ) = 0;
      virtual int unsubscribe (std::string const &id) = 0;
    };
  }
}

#endif
