#ifndef GSPC_NET_SERVER_BASE_CLIENT_HPP
#define GSPC_NET_SERVER_BASE_CLIENT_HPP

#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/frame_handler.hpp>
#include <gspc/net/parse/parser.hpp>

#include <gspc/net/client.hpp>
#include <gspc/net/common/base_connection.hpp>

namespace gspc
{
  namespace net
  {
    namespace client
    {
      template <class Protocol>
      class base_client : public gspc::net::client_t
                        , public gspc::net::frame_handler_t
                        , private boost::noncopyable
      {
      public:
        typedef Protocol                         protocol_type;
        typedef typename protocol_type::socket   socket_type;
        typedef typename protocol_type::endpoint endpoint_type;
        typedef typename server::base_connection<protocol_type> connection_type;
        typedef boost::shared_ptr<connection_type> connection_ptr_t;

        explicit
        base_client (endpoint_type const &);
        ~base_client ();

        int start ();
        int stop ();

        void set_frame_handler (frame_handler_t &);

        int send_raw (frame const &);

        int send (frame const &);
        int request (frame const &rqst, frame &rply);
        int subscribe ( std::string const &dest
                      , std::string const &id
                      );
        int unsubscribe (std::string const &id);

        int handle_frame (user_ptr, frame const &);
        int handle_error (user_ptr, boost::system::error_code const &);
      private:
        boost::asio::io_service         m_io_service;
        endpoint_type                   m_endpoint;
        connection_ptr_t                m_connection;

        frame_handler_t                *m_frame_handler;

        typedef boost::shared_ptr<boost::thread> thread_ptr_t;
        typedef std::vector<thread_ptr_t>        thread_pool_t;
        size_t                                   m_thread_pool_size;
        thread_pool_t                            m_thread_pool;
      };
    }
  }
}

#endif
