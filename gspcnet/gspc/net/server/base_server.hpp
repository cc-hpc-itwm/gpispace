#ifndef GSPC_NET_SERVER_BASE_SERVER_HPP
#define GSPC_NET_SERVER_BASE_SERVER_HPP

#include <string>
#include <vector>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

#include <gspc/net/server.hpp>
#include <gspc/net/frame_handler.hpp>
#include <gspc/net/common/base_connection.hpp>
#include <gspc/net/server/queue_manager_fwd.hpp>

namespace gspc
{
  namespace net
  {
    namespace server
    {
      template <class Protocol>
      class base_server : public gspc::net::server_t
                        , public gspc::net::frame_handler_t
                        , public boost::enable_shared_from_this<base_server<Protocol> >
                        , private boost::noncopyable
      {
      public:
        typedef Protocol                         protocol_type;
        typedef typename protocol_type::endpoint endpoint_type;
        typedef typename protocol_type::acceptor acceptor_type;

        base_server ( boost::asio::io_service &
                    , endpoint_type const &
                    , queue_manager_t &qmgr
                    );
        ~base_server ();

        int start ();
        int stop ();
        std::string url () const;

        int handle_frame (user_ptr, frame const &);
        int handle_error (user_ptr, boost::system::error_code const &);

        void set_queue_length (size_t);
      private:
        typedef base_connection<protocol_type> connection;
        typedef boost::shared_ptr<connection>  connection_ptr;

        void start_accept ();
        void handle_accept (boost::system::error_code const &);

        queue_manager_t & m_qmgr;

        boost::asio::io_service &m_io_service;
        boost::asio::io_service::strand m_strand;
        endpoint_type           m_endpoint;
        acceptor_type           m_acceptor;
        connection_ptr          m_new_connection;

        size_t m_queue_length;
      };
    }
  }
}

#endif
