#ifndef GSPC_NET_FRAME_BUILDER_HPP
#define GSPC_NET_FRAME_BUILDER_HPP

#include <string>

#include <gspc/net/error.hpp>
#include <gspc/net/frame.hpp>
#include <gspc/net/header.hpp>

namespace gspc
{
  namespace net
  {
    namespace make
    {
      frame const & heartbeat_frame ();

      // server -> client frames
      frame error_frame (frame const &, error_code_t ec, std::string const&);
      frame error_frame (error_code_t ec, const char *message);
      frame error_frame (error_code_t ec, std::string const &message);

      frame receipt_frame (gspc::net::header::receipt const &);
      frame connected_frame (gspc::net::header::version const &);
      frame message_frame (frame const & send_frame);

      frame send_frame ( gspc::net::header::destination const &
                       , const char *data, size_t len
                       );
      frame subscribe_frame ( gspc::net::header::destination const &
                            , gspc::net::header::id const &
                            );
      frame unsubscribe_frame (gspc::net::header::id const &);

      frame ack_frame (gspc::net::header::id const &);
      frame nack_frame (gspc::net::header::id const &);

      // transaction related
      frame begin_frame (gspc::net::header::transaction const &);
      frame commit_frame (gspc::net::header::transaction const &);
      frame abort_frame (gspc::net::header::transaction const &);

      frame connect_frame ();
      frame disconnect_frame ();
    }
  }
}

#endif
