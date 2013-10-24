// -*- mode: c++ -*-
#include "base_connection.hpp"

#include <boost/bind.hpp>

#include <gspc/net/frame_io.hpp>
#include <gspc/net/frame_util.hpp>
#include <gspc/net/frame_handler.hpp>

#include <iostream>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Proto>
      base_connection<Proto>::base_connection ( size_t id
                                              , boost::asio::io_service &service
                                              , frame_handler_t & frame_handler
                                              )
        : m_pending_mutex ()
        , m_frame_handler (frame_handler)
        , m_strand (service)
        , m_socket (service)
        , m_buffer ()
        , m_parser ()
        , m_frame ()
        , m_pending ()
        , m_shutting_down (false)
        , m_max_queue_length (0)
        , m_id (id)
      {}

      template <class Proto>
      base_connection<Proto>::~base_connection ()
      {
        if (m_socket.is_open ())
          stop ();
      }

      template <class Proto>
      typename base_connection<Proto>::socket_type &
      base_connection<Proto>::socket ()
      {
        return m_socket;
      }

      template <class Proto>
      void base_connection<Proto>::set_queue_length (size_t len)
      {
        m_max_queue_length = len;
      }

      template <class Proto>
      void base_connection<Proto>::start ()
      {
        unique_lock lock (m_shutting_down_mutex);
        m_shutting_down = false;

        m_socket.async_read_some
          ( boost::asio::buffer (m_buffer)
          , m_strand.wrap (boost::bind
                          ( &base_connection<Proto>::handle_read
                          , this->shared_from_this ()
                          , boost::asio::placeholders::error
                          , boost::asio::placeholders::bytes_transferred
                          ))
          );
      }

      template <class Proto>
      void base_connection<Proto>::stop ()
      {
        {
          //  unique_lock lock (m_shutting_down_mutex);
          m_shutting_down = true;
        }

        boost::system::error_code ec;
        m_socket.cancel (ec);
        m_socket.shutdown (Proto::socket::shutdown_both, ec);
        m_socket.close (ec);
      }

      template <class Proto>
      size_t base_connection<Proto>::id () const
      {
        return m_id;
      }

      template <class Proto>
      int base_connection<Proto>::deliver (frame const &f)
      {
        {
          unique_lock lock (m_shutting_down_mutex);
          if (m_shutting_down)
            return -ESHUTDOWN;
        }

        unique_lock lock (m_pending_mutex);

        if (m_max_queue_length && m_pending.size () >= m_max_queue_length)
          return -ENOBUFS;

        const bool write_in_progress = not m_pending.empty ();
        m_pending.push_back (f);

        if (not write_in_progress)
        {
          boost::asio::async_write
            ( m_socket
            , boost::asio::buffer (m_pending.front ().to_string ())
            , boost::bind ( &base_connection<Proto>::handle_write
                          , this->shared_from_this ()
                          , boost::asio::placeholders::error
                          )
            );
        }

        return 0;
      }

      template <class Proto>
      void
      base_connection<Proto>::handle_read ( const boost::system::error_code & ec
                                          , std::size_t transferred
                                          )
      {
        if (not ec)
        {
          gspc::net::parse::result_t result;

          std::size_t offset = 0;
          std::size_t remaining = transferred;

          while (remaining)
          {
            result = m_parser.parse ( m_buffer.data () + offset
                                    , m_buffer.data () + offset + remaining
                                    , m_frame
                                    );

            if (result.state == gspc::net::parse::PARSE_FAILED)
            {
              return;
            }

            remaining -= result.consumed;
            offset += result.consumed;

            if (result.state == gspc::net::parse::PARSE_FINISHED)
            {
              m_parser.reset ();

              if (not is_heartbeat (m_frame))
              {
                int error = this->m_frame_handler.handle_frame (this, m_frame);
                if (error < 0)
                {
                  return;
                }
              }

              m_frame = frame ();
            }
          }

          m_socket.async_read_some
            ( boost::asio::buffer (m_buffer)
            , m_strand.wrap (boost::bind
                            ( &base_connection<Proto>::handle_read
                            , this->shared_from_this ()
                            , boost::asio::placeholders::error
                            , boost::asio::placeholders::bytes_transferred
                            ))
            );
        }
        else
        {
          unique_lock lock (m_shutting_down_mutex);
          if (not m_shutting_down)
          {
            this->m_frame_handler.handle_error (this, ec);
          }
        }
      }

      template <class Proto>
      void
      base_connection<Proto>::handle_write (const boost::system::error_code &ec)
      {
        if (not ec)
        {
          unique_lock lock (m_pending_mutex);
          m_pending.pop_front ();

          if (not m_pending.empty ())
          {
            boost::asio::async_write
              ( m_socket
              , boost::asio::buffer (m_pending.front ().to_string ())
              , boost::bind ( &base_connection<Proto>::handle_write
                            , this->shared_from_this ()
                            , boost::asio::placeholders::error
                            )
              );
          }
        }
        else
        {
          unique_lock lock (m_shutting_down_mutex);
          if (not m_shutting_down)
          {
            this->m_frame_handler.handle_error (this, ec);
          }
        }
      }
    }
  }
}
