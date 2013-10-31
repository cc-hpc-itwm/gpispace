// -*- mode: c++ -*-
#include "base_client.hpp"

#include <fhg/assert.hpp>

#include <boost/format.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <gspc/net/frame_io.hpp>
#include <gspc/net/frame_builder.hpp>
#include <gspc/net/client/dummy_frame_handler.hpp>
#include <gspc/net/client/response.hpp>

#include <gspc/net/auth/cookie.hpp>

namespace gspc
{
  namespace net
  {
    namespace client
    {
      enum state_t
        {
          DISCONNECTED
        , CONNECTED
        , FAILED
        };

      template <class Proto>
      base_client<Proto>::base_client ( boost::asio::io_service &io
                                      , endpoint_type const &endpoint
                                      )
        : m_io_service (io)
        , m_endpoint (endpoint)
        , m_state (DISCONNECTED)
        , m_connection ()
        , m_frame_handler (&dummy_frame_handler ())
        , m_message_id (0)
        , m_responses_mutex ()
        , m_responses ()
        , m_timeout (boost::posix_time::pos_infin)
        , m_last_error_code ()
      {}

      template <class Proto>
      base_client<Proto>::~base_client ()
      {
        stop ();
      }

      template <class Proto>
      int base_client<Proto>::start ()
      {
        return this->start (m_timeout);
      }

      template <class Proto>
      int base_client<Proto>::start (const boost::posix_time::time_duration timeout)
      {
        boost::system::error_code ec;

        {
          unique_lock _ (m_mutex);
          m_connection.reset (new connection_type (0, m_io_service, *this));
          m_connection->socket ().connect (m_endpoint, ec);
        }

        if (not ec)
        {
          m_connection->start ();
          return this->connect (timeout);
        }
        else
        {
          return -ECONNREFUSED;
        }
      }

      template <class Proto>
      int base_client<Proto>::stop ()
      {
        unique_lock _ (m_mutex);

        if (m_connection)
        {
          m_connection->stop ();
        }

        return 0;
      }

      template <class Proto>
      bool
      base_client<Proto>::is_connected () const
      {
        shared_lock _ (m_mutex);
        return this->m_state == CONNECTED;
      }

      template <class Proto>
      boost::system::error_code const &
      base_client<Proto>::last_error_code () const
      {
        return this->m_last_error_code;
      }

      template <class Proto>
      std::string const &
      base_client<Proto>::get_private_queue () const
      {
        return this->m_priv_queue;
      }

      template <class Proto>
      void
      base_client<Proto>::set_timeout (size_t ms)
      {
        if (std::size_t (-1) == ms)
        {
          m_timeout = boost::posix_time::pos_infin;
        }
        else
        {
          m_timeout = boost::posix_time::milliseconds (ms);
        }
      }

      template <class Proto>
      int base_client<Proto>::connect ()
      {
        return this->connect (m_timeout);
      }

      template <class Proto>
      int base_client<Proto>::connect (const boost::posix_time::time_duration timeout)
      {
        frame rply;
        frame cnct;
        int rc;

        cnct = make::connect_frame ();
        header::set (cnct, "cookie", auth::get_cookie ());

        rc = send_and_wait ( cnct
                           , rply
                           , timeout
                           );

        if (rc != 0)
        {
          {
            unique_lock _ (m_mutex);
            m_state = FAILED;
          }

          m_connection.reset ();

          return rc;
        }

        if (rply.get_command () == "CONNECTED")
        {
          std::string session_id =
            header::get (rply, "session-id", std::string ());

          m_priv_queue =
            ( boost::format ("/queue/%1%-%2%/replies")
            % session_id
            % getpid ()
            ).str ();


          {
            unique_lock _ (m_mutex);
            m_state = CONNECTED;
          }

          subscribe (m_priv_queue, m_priv_queue);

          return 0;
        }
        else if (rply.get_command () == "ERROR")
        {
          {
            unique_lock _ (m_mutex);
            m_state = FAILED;
          }
          return header::get (rply, "code", -EPERM);
        }
        else
        {
          {
            unique_lock _ (m_mutex);
            m_state = FAILED;
          }
          return -EPROTO;
        }
      }

      template <class Proto>
      int base_client<Proto>::disconnect ()
      {
        return send_raw (make::disconnect_frame ());
      }

      template <class Proto>
      void base_client<Proto>::set_frame_handler (frame_handler_t &h)
      {
        this->m_frame_handler = &h;
      }

      template <class Proto>
      int base_client<Proto>::send_raw (frame const &f)
      {
        if (m_connection)
        {
          return m_connection->deliver (f);
        }
        else
        {
          return -ENOTCONN;
        }
      }

      template <class Proto>
      int base_client<Proto>::send ( std::string const & dst
                                   , std::string const & body
                                   )
      {
        return send_raw
          (make::send_frame ( header::destination (dst)
                            , body.c_str ()
                            , body.size ()
                            )
          );
      }

      template <class Proto>
      int base_client<Proto>::send_sync ( std::string const & dst
                                        , std::string const & body
                                        , const boost::posix_time::time_duration to_wait
                                        )
      {
        frame f_send ("SEND");
        f_send.set_body (body);
        f_send.set_header ("destination", dst);

        return this->send_sync (f_send, to_wait);
      }

      template <class Proto>
      int base_client<Proto>::send_sync ( frame const &f
                                        , const boost::posix_time::time_duration to_wait
                                        )
      {
        int rc = 0;

        response_ptr receipt
          (new response_t ( "receipt-"
                          + boost::lexical_cast<std::string>(++m_message_id)
                          ));
        frame to_send (f);
        gspc::net::header::receipt (receipt->id ()).apply_to (to_send);

        {
          unique_lock lock (m_responses_mutex);
          m_responses [receipt->id ()] = receipt;
        }

        rc = send_raw (to_send);
        if (0 == rc)
        {
          rc = receipt->wait (to_wait);
        }

        {
          unique_lock lock (m_responses_mutex);
          m_responses.erase (receipt->id ());
        }

        if (rc == 0)
        {
          if (receipt->get_reply ())
          {
            frame const & rply = *receipt->get_reply ();
            if (rply.get_command () == "RECEIPT")
            {
              rc = 0;
            }
            else if (rply.get_command () == "ERROR")
            {
              rc = header::get (rply, "code", -EPERM);
            }
            else
            {
              rc = -EPROTO;
            }
          }
          else
          {
            rc = -ETIME;
          }
        }

        return rc;
      }

      template <class Proto>
      int base_client<Proto>::send_and_wait ( frame const &rqst
                                            , frame &rply
                                            , const boost::posix_time::time_duration to_wait
                                            )
      {
        int rc = 0;

        response_ptr response
          (new response_t ( "request-"
                          + boost::lexical_cast<std::string>(++m_message_id)
                          ));

        frame to_send (rqst);
        gspc::net::header::message_id (response->id ()).apply_to (to_send);

        {
          unique_lock lock (m_responses_mutex);
          m_responses [response->id ()] = response;
        }

        rc = send_raw (to_send);
        if (0 == rc)
        {
          rc = response->wait (to_wait);
        }

        {
          unique_lock lock (m_responses_mutex);
          m_responses.erase (response->id ());
        }

        if (rc == 0)
        {
          if (response->get_reply ())
          {
            rply = *response->get_reply ();
          }
          else
          {
            return -ETIME;
          }
        }

        return rc;
      }

      template <class Proto>
      int base_client<Proto>::request ( std::string const &dst
                                      , std::string const &body
                                      , frame &rply
                                      , const boost::posix_time::time_duration t
                                      )
      {
        frame rqst ("SEND");
        rqst.set_body (body);
        rqst.set_header ("destination", dst);

        return this->request (rqst, rply, t);
      }

      template <class Proto>
      int base_client<Proto>::request ( frame const &f
                                      , frame &rply
                                      )
      {
        return this->request (f, rply, m_timeout);
      }

      template <class Proto>
      int base_client<Proto>::request ( frame const &f
                                      , frame &rply
                                      , const boost::posix_time::time_duration t
                                      )
      {
        int rc;
        frame rqst (f);
        rqst.set_command ("SEND");
        rqst.set_header ("reply-to", m_priv_queue);

        rc = send_and_wait (rqst, rply, t);

        return rc;
      }

      template <class Proto>
      int base_client<Proto>::subscribe ( std::string const &dst
                                        , std::string const &id
                                        )
      {
        if (m_state != CONNECTED)
        {
          return -ENOTCONN;
        }

        frame f_sub = make::subscribe_frame ( header::destination (dst)
                                            , header::id (id)
                                            );
        return send_sync (f_sub, m_timeout);
      }

      template <class Proto>
      int base_client<Proto>::unsubscribe (std::string const &id)
      {
        if (m_state != CONNECTED)
        {
          return -ENOTCONN;
        }

        return send_raw (make::unsubscribe_frame (header::id (id)));
      }

      template <class Proto>
      bool base_client<Proto>::try_notify_response (frame const &f)
      {
        std::string id;
        if (frame::header_value h = f.get_header ("receipt-id"))
        {
          id = *h;
        }
        else if (frame::header_value h = f.get_header ("correlation-id"))
        {
          id = *h;
        }

        shared_lock lock (m_responses_mutex);
        const response_map_t::iterator response_it = m_responses.find (id);
        if (response_it != m_responses.end ())
        {
          response_it->second->notify (f);
          return true;
        }
        else
        {
          return false;
        }
      }

      template <class Proto>
      size_t
      base_client<Proto>::abort_all_responses (boost::system::error_code const &ec)
      {
        size_t count = 0;

        shared_lock lock (m_responses_mutex);
        BOOST_FOREACH (response_map_t::value_type resp, m_responses)
        {
          resp.second->abort (ec);
          ++count;
        }

        return count;
      }

      template <class Proto>
      int base_client<Proto>::handle_frame (user_ptr user, frame const &f)
      {
        if (try_notify_response (f))
          return 0;

        onFrame (f);

        return this->m_frame_handler->handle_frame (user, f);
      }

      template <class Proto>
      int base_client<Proto>::handle_error ( user_ptr user
                                           , boost::system::error_code const &ec
                                           )
      {
        {
          unique_lock _ (m_mutex);
          m_state = FAILED;
        }

        if (0 == abort_all_responses (ec))
        {
          onError (ec);
          return m_frame_handler->handle_error (user, ec);
        }

        return 0;
      }
    }
  }
}
