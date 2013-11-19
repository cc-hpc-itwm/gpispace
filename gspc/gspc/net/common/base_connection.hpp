#ifndef GSPC_NET_SERVER_BASE_CONNECTION_HPP
#define GSPC_NET_SERVER_BASE_CONNECTION_HPP

#include <deque>

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
        base_connection ( size_t id
                        , boost::asio::io_service & io_service
                        , frame_handler_t & frame_handler
                        );

        ~base_connection ();

        socket_type & socket ();

        void start ();
        void stop ();

        int deliver (frame const &);
        size_t id () const;

        void set_queue_length (size_t);
        void set_heartbeat_info (heartbeat_info_t const &);
      private:
        typedef boost::lock_guard<boost::mutex> unique_lock;

        typedef std::deque<frame>  frame_list_t;

        void start_heartbeats ();
        void handle_read ( const boost::system::error_code &
                         , std::size_t transferred
                         );
        void handle_write (const boost::system::error_code &);

        void handle_recv_heartbeat_timer (const boost::system::error_code &);
        void handle_send_heartbeat_timer (const boost::system::error_code &);

        mutable boost::mutex     m_shutting_down_mutex;
        mutable boost::mutex     m_pending_mutex;

        frame_handler_t &m_frame_handler;

        boost::asio::io_service::strand m_strand;
        socket_type                     m_socket;

        boost::array<char, 8192> m_buffer;

        parse::parser m_parser;
        frame         m_frame;
        frame_list_t  m_pending;

        bool   m_shutting_down;
        size_t m_max_queue_length;

        size_t m_id;

        mutable boost::mutex m_heartbeat_mutex;

        boost::posix_time::ptime m_recv_timestamp;
        boost::posix_time::ptime m_send_timestamp;

        boost::asio::deadline_timer m_recv_heartbeat_timer;
        boost::asio::deadline_timer m_send_heartbeat_timer;

        heartbeat_info_t m_heartbeat_info;
      };
    }
  }
}

#endif
