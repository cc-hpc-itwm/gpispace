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
      base_connection<Proto>::base_connection ( boost::asio::io_service &service
                                              , frame_handler_t & frame_handler
                                              )
        : m_frame_list_mutex ()
        , m_frame_handler (frame_handler)
        , m_strand (service)
        , m_socket (service)
        , m_buffer ()
        , m_parser ()
        , m_frame ()
        , m_frame_list ()
      {}

      template <class Proto>
      base_connection<Proto>::~base_connection ()
      {}

      template <class Proto>
      typename base_connection<Proto>::socket_type &
      base_connection<Proto>::socket ()
      {
        return m_socket;
      }

      template <class Proto>
      void base_connection<Proto>::start ()
      {
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
      int base_connection<Proto>::deliver (frame const &f)
      {
        unique_lock lock (m_frame_list_mutex);

        bool write_in_progress = not m_frame_list.empty ();
        m_frame_list.push_back (f);
        m_frame_list.back ().close ();
        m_buffer_list.push_back (m_frame_list.back ().to_string ());

        if (not write_in_progress)
        {
          m_socket.async_send
            ( boost::asio::buffer (m_buffer_list.front ())
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
                int error = m_frame_handler.handle_frame (this, m_frame);
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
          m_frame_handler.handle_error (this, ec);
        }
      }

      template <class Proto>
      void
      base_connection<Proto>::handle_write (const boost::system::error_code &ec)
      {
        if (not ec)
        {
          unique_lock lock (m_frame_list_mutex);
          m_frame_list.pop_front ();
          m_buffer_list.pop_front ();

          if (not m_buffer_list.empty ())
          {
            m_socket.async_send
              ( boost::asio::buffer (m_buffer_list.front ())
              , boost::bind ( &base_connection<Proto>::handle_write
                            , this->shared_from_this ()
                            , boost::asio::placeholders::error
                            )
              );
          }
        }
        else
        {
          m_frame_handler.handle_error (this, ec);
        }
      }
    }
  }
}
