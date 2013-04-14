#ifndef GSPC_NET_SERVER_BASE_CONNECTION_HPP
#define GSPC_NET_SERVER_BASE_CONNECTION_HPP

#include <list>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/frame_handler_fwd.hpp>
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
        base_connection ( boost::asio::io_service & io_service
                        , frame_handler_t & frame_handler
                        );

        ~base_connection ();

        socket_type & socket ();

        void start ();

        int deliver (frame const &);
      private:
        typedef boost::lock_guard<boost::mutex> unique_lock;

        typedef std::list<frame>        frame_list_t;
        typedef std::list<std::string>  buffer_list_t;

        void handle_read ( const boost::system::error_code &
                         , std::size_t transferred
                         );
        void handle_write (const boost::system::error_code &);

        mutable boost::mutex     m_frame_list_mutex;

        frame_handler_t &m_frame_handler;

        boost::asio::io_service::strand m_strand;
        socket_type                     m_socket;

        boost::array<char, 8192> m_buffer;

        parse::parser m_parser;
        frame         m_frame;
        frame_list_t  m_frame_list;
        buffer_list_t m_buffer_list;
      };
    }
  }
}

#endif
