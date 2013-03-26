#include "tcp_connection.hpp"

#include <boost/bind.hpp>

#include <gspc/net/frame_io.hpp>

#include <iostream>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      tcp_connection::tcp_connection ( boost::asio::io_service & io_service
                                     , tcp_server *owner
                                     , queue_manager_t & qmgr
                                     )
        : m_strand (io_service)
        , m_socket (io_service)
        , m_buffer ()
        , m_parser ()
        , m_frame ()
        , m_owner (owner)
        , m_qmgr (qmgr)
      {}

      tcp_connection::~tcp_connection ()
      {
        // m_owner->handle_connection_close () ?
      }

      boost::asio::ip::tcp::socket & tcp_connection::socket ()
      {
        return m_socket;
      }

      void tcp_connection::start ()
      {
        std::cerr << "new connection: "
                  << m_socket.remote_endpoint ()
                  << std::endl
          ;

        m_socket.async_read_some
          ( boost::asio::buffer (m_buffer)
          , m_strand.wrap (boost::bind
                          ( &tcp_connection::handle_read
                          , shared_from_this ()
                          , boost::asio::placeholders::error
                          , boost::asio::placeholders::bytes_transferred
                          ))
          );
      }

      int tcp_connection::deliver (frame const &f)
      {
        std::cerr << "delivering " << f << std::endl;
        return 0;
      }

      void tcp_connection::handle_read ( const boost::system::error_code & ec
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

            if (result.state == gspc::net::parse::PARSE_FINISHED)
            {
              remaining -= result.consumed;
              offset += result.consumed;

              m_parser.reset ();

              std::cerr << "got frame: " << m_frame;

              m_frame = frame ();
            }
          }

          m_socket.async_read_some
            ( boost::asio::buffer (m_buffer)
            , m_strand.wrap (boost::bind
                            ( &tcp_connection::handle_read
                            , shared_from_this ()
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

      void tcp_connection::handle_write (const boost::system::error_code &ec)
      {

      }
    }
  }
}
