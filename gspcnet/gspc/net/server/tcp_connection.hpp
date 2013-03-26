#ifndef GSPC_NET_SERVER_TCP_CONNECTION_HPP
#define GSPC_NET_SERVER_TCP_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/parse/parser.hpp>

#include <gspc/net/server/user.hpp>
#include <gspc/net/server/queue_manager_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      class tcp_server;

      class tcp_connection
        : public boost::enable_shared_from_this<tcp_connection>
        , public gspc::net::server::user_t
        , private boost::noncopyable
      {
      public:
        tcp_connection ( boost::asio::io_service & io_service
                       , tcp_server *owner
                       , queue_manager_t & qmgr
                       );

        ~tcp_connection ();

        boost::asio::ip::tcp::socket & socket ();

        void start ();

        int deliver (frame const &);
      private:
        void handle_read ( const boost::system::error_code &
                         , std::size_t transferred
                         );
        void handle_write (const boost::system::error_code &);

        boost::asio::io_service::strand m_strand;
        boost::asio::ip::tcp::socket    m_socket;

        boost::array<char, 8192> m_buffer;

        parse::parser m_parser;
        frame        m_frame;

        tcp_server        *m_owner;
        queue_manager_t &  m_qmgr;
      };

      typedef boost::shared_ptr<tcp_connection> tcp_connection_ptr;
    }
  }
}

#endif
