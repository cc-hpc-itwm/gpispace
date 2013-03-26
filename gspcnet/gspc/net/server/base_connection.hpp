#ifndef GSPC_NET_SERVER_BASE_CONNECTION_HPP
#define GSPC_NET_SERVER_BASE_CONNECTION_HPP

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/parse/parser.hpp>

#include <gspc/net/user.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Protocol>
      class base_connection
        : public boost::enable_shared_from_this<base_connection<Protocol> >
        , public gspc::net::user_t
        , private boost::noncopyable
      {
      public:
        typedef Protocol protocol_type;
        typedef typename protocol_type::socket socket_type;

        explicit
        base_connection (boost::asio::io_service & io_service);

        ~base_connection ();

        socket_type & socket ();

        void start ();

        int deliver (frame const &);
      private:
        void handle_read ( const boost::system::error_code &
                         , std::size_t transferred
                         );
        void handle_write (const boost::system::error_code &);

        boost::asio::io_service::strand m_strand;
        socket_type                     m_socket;

        boost::array<char, 8192> m_buffer;

        parse::parser m_parser;
        frame         m_frame;
      };
    }
  }
}

#endif
