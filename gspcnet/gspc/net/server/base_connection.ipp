// -*- mode: c++ -*-
#include "base_connection.hpp"

#include <boost/bind.hpp>

#include <gspc/net/frame_io.hpp>
#include <gspc/net/frame_buffer.hpp>
#include <gspc/net/frame_util.hpp>

#include <iostream>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Proto>
      base_connection<Proto>::base_connection (boost::asio::io_service &service)
        : m_frame_list_mutex ()
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

        if (not write_in_progress)
        {
          m_socket.async_send
            ( frame_to_buffers (m_frame_list.front ())
            , m_strand.wrap (boost::bind
                            ( &base_connection<Proto>::handle_write
                            , this->shared_from_this ()
                            , boost::asio::placeholders::error
                            ))
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
              std::cerr << "parsing failed: "
                        << gspc::net::error_string (result.reason)
                        << std::endl
                ;
              std::cerr << m_frame.to_hex () << std::endl;
              return;
            }

            remaining -= result.consumed;
            offset += result.consumed;

            if (result.state == gspc::net::parse::PARSE_FINISHED)
            {
              m_parser.reset ();

              if (not is_heartbeat (m_frame))
              {
                this->deliver (m_frame);
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
          std::cerr << "handle connection lost event" << std::endl;
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

          if (not m_frame_list.empty ())
          {
            m_socket.async_send
              ( frame_to_buffers (m_frame_list.front ())
              , m_strand.wrap (boost::bind
                              ( &base_connection<Proto>::handle_write
                              , this->shared_from_this ()
                              , boost::asio::placeholders::error
                              ))
              );
          }
        }
        else
        {
          std::cerr << "handle write error (disconnect)" << std::endl;
        }
      }
    }
  }
}
