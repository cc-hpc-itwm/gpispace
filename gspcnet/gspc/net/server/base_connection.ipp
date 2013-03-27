// -*- mode: c++ -*-
#include "base_connection.hpp"

#include <boost/bind.hpp>

#include <gspc/net/frame_io.hpp>

#include <iostream>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Proto>
      base_connection<Proto>::base_connection (boost::asio::io_service &service)
        : m_strand (service)
        , m_socket (service)
        , m_buffer ()
        , m_parser ()
        , m_frame ()
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
        std::cerr << "new connection: "
                  << m_socket.remote_endpoint ()
                  << " -> "
                  << m_socket.local_endpoint ()
                  << std::endl
          ;

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
        std::cerr << "delivering " << f << std::endl;
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
              return;
            }

            remaining -= result.consumed;
            offset += result.consumed;

            if (result.state == gspc::net::parse::PARSE_FINISHED)
            {
              m_parser.reset ();

              std::cerr << "got frame: " << m_frame;

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
          std::cerr << "lost connection" << std::endl;
        }
      }

      template <class Proto>
      void
      base_connection<Proto>::handle_write (const boost::system::error_code &ec)
      {

      }
    }
  }
}
