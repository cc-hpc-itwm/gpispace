// -*- mode: c++ -*-
#include "base_client.hpp"

#include <fhg/assert.hpp>

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
      base_client<Proto>::base_client (endpoint_type const &endpoint)
        : m_io_service ()
        , m_endpoint (endpoint)
        , m_state (DISCONNECTED)
        , m_connection ()
        , m_frame_handler (&dummy_frame_handler ())
        , m_thread_pool_size (1)
        , m_thread_pool ()
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
        assert (m_thread_pool.empty ());

        m_io_service.reset ();

        m_connection.reset (new connection_type (m_io_service, *this));
        m_connection->socket ().connect (m_endpoint);
        m_connection->start ();

        for (size_t i = 0 ; i < m_thread_pool_size ; ++i)
        {
          thread_ptr_t thrd
            (new boost::thread (boost::bind ( &boost::asio::io_service::run
                                            , &m_io_service
                                            )
                               )
            );
          m_thread_pool.push_back (thrd);
        }

        return 0;
      }

      template <class Proto>
      int base_client<Proto>::stop ()
      {
        m_io_service.stop ();

        BOOST_FOREACH (thread_ptr_t thrd, m_thread_pool)
        {
          thrd->join ();
        }
        m_thread_pool.clear ();

        return 0;
      }

      template <class Proto>
      boost::system::error_code const &
      base_client<Proto>::last_error_code () const
      {
        return this->m_last_error_code;
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
        frame rply;
        frame cnct;
        int rc;

        cnct = make::connect_frame ();
        header::set (cnct, "cookie", auth::get_cookie ());

        rc = send_and_wait ( cnct
                           , rply
                           , m_timeout
                           );

        if (rc != 0)
        {
          m_state = FAILED;
          return rc;
        }

        if (rply.get_command () == "CONNECTED")
        {
          m_state = CONNECTED;
          return 0;
        }
        else if (rply.get_command () == "ERROR")
        {
          m_state = FAILED;
          return header::get (rply, "code", -EPERM);
        }
        else
        {
          m_state = FAILED;
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
        m_frame_handler = &h;
      }

      template <class Proto>
      int base_client<Proto>::send_raw (frame const &f)
      {
        return m_connection->deliver (f);
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
        int rc = 0;

        response_ptr receipt
          (new response_t ( "message-"
                          + boost::lexical_cast<std::string>(++m_message_id)
                          ));

        frame to_send (make::send_frame ( header::destination (dst)
                                        , body.c_str ()
                                        , body.size ()
                                        )
                      );
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
            frame rply = *receipt->get_reply ();
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
          (new response_t ( "message-"
                          + boost::lexical_cast<std::string>(++m_message_id)
                          ));

        frame to_send (rqst);
        gspc::net::header::receipt (response->id ()).apply_to (to_send);
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
        frame rqst ("REQUEST");
        rqst.set_body (body);
        rqst.set_header ("destination", dst);

        return this->request (rqst, rply, t);
      }

      template <class Proto>
      int base_client<Proto>::request ( frame const &f
                                      , frame &rply
                                      , const boost::posix_time::time_duration t
                                      )
      {
        int rc;
        frame rqst (f);
        rqst.set_command ("REQUEST");

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
        frame f_rep;
        return send_and_wait (f_sub, f_rep, m_timeout);
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
      bool base_client<Proto>::try_notify_response ( std::string const &id
                                                   , frame const &f
                                                   )
      {
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
      int base_client<Proto>::handle_frame (user_ptr user, frame const &f)
      {
        if (  f.has_header ("receipt-id")
           && try_notify_response (*f.get_header ("receipt-id"), f)
           )
        {
          return 0;
        }
        else if (  f.has_header ("correlation-id")
                && try_notify_response (*f.get_header ("correlation-id"), f)
                )
        {
          return 0;
        }

        return m_frame_handler->handle_frame (user, f);
      }

      template <class Proto>
      int base_client<Proto>::handle_error ( user_ptr user
                                           , boost::system::error_code const &ec
                                           )
      {
        m_state = FAILED;
        return m_frame_handler->handle_error (user, ec);
      }
    }
  }
}
