#include "frame_builder.hpp"

#include <boost/lexical_cast.hpp>

namespace gspc
{
  namespace net
  {
    namespace make
    {
      frame error_frame ( error_code_t ec
                        , const char *message
                        )
      {
        return error_frame (ec, std::string (message));
      }

      frame error_frame ( error_code_t ec
                        , std::string const &message
                        )
      {
        frame f ("ERROR");
        f.set_header ("content-type", "text/plain");
        f.set_header ("code", boost::lexical_cast<std::string>(ec));
        f.set_header ("message", gspc::net::error_string (ec));
        f.add_body (message);
        return f;
      }

      frame error_frame ( frame const &f
                        , error_code_t ec
                        , std::string const &message
                        )
      {
        frame error = error_frame (ec, message);
        if (frame::header_value h = f.get_header ("receipt"))
        {
          gspc::net::header::receipt_id r_id (*h);
          r_id.apply_to (error);
        }
        if (frame::header_value h = f.get_header ("reply-to"))
        {
          header::set (error, "destination", *h);
        }
        return error;
      }

      frame receipt_frame (gspc::net::header::receipt const &id)
      {
        frame f ("RECEIPT");
        gspc::net::header::receipt_id r_id (*id.value ());
        r_id.apply_to (f);
        return f;
      }

      frame connected_frame (gspc::net::header::version const &version)
      {
        frame f ("CONNECTED");
        version.apply_to (f);
        return f;
      }

      frame message_frame (frame const & send_frame)
      {
        frame f (send_frame);
        f.set_command ("MESSAGE");
        return f;
      }

      frame reply_frame (frame const & send_frame)
      {
        frame f (send_frame);
        f.set_command ("MESSAGE");
        f.set_header ("destination", send_frame.get_header ("reply-to"));

        if (frame::header_value h = send_frame.get_header ("receipt"))
        {
          gspc::net::header::receipt_id r_id (*h);
          r_id.apply_to (f);
        }
        if (frame::header_value h = send_frame.get_header ("message-id"))
        {
          f.set_header ("correlation-id", *h);
        }

        return f;
      }

      frame send_frame ( gspc::net::header::destination const & dst
                       , const char *body, size_t len
                       )
      {
        frame f ("SEND", body, len);
        dst.apply_to (f);
        return f;
      }

      frame subscribe_frame ( gspc::net::header::destination const &dst
                            , gspc::net::header::id const &id
                            )
      {
        frame f ("SUBSCRIBE");
        dst.apply_to (f);
        id.apply_to (f);
        return f;
      }

      frame unsubscribe_frame (gspc::net::header::id const &id)
      {
        frame f ("UNSUBSCRIBE");
        id.apply_to (f);
        return f;
      }

      frame ack_frame (gspc::net::header::id const &id)
      {
        frame f ("ACK");
        id.apply_to (f);
        return f;
      }

      frame nack_frame (gspc::net::header::id const &id)
      {
        frame f ("NACK");
        id.apply_to (f);
        return f;
      }

      frame begin_frame (gspc::net::header::transaction const &trans)
      {
        frame f ("BEGIN");
        trans.apply_to (f);
        return f;
      }

      frame commit_frame (gspc::net::header::transaction const &trans)
      {
        frame f ("COMMIT");
        trans.apply_to (f);
        return f;
      }

      frame abort_frame (gspc::net::header::transaction const &trans)
      {
        frame f ("ABORT");
        trans.apply_to (f);
        return f;
      }

      frame connect_frame ()
      {
        frame f ("CONNECT");
        //f.set_header ("login", "...");
        //f.set_header ("passcode", "...");
        //f.set_header ("heart-beat", "1000,1000");
        //f.set_header ("accept-version","1.0,1.2");
        return f;
      }

      frame disconnect_frame ()
      {
        frame f ("DISCONNECT");
        return f;
      }

      frame const & heartbeat_frame ()
      {
        static frame f;
        return f;
      }
    }
  }
}
