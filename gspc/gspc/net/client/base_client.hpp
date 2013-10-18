#ifndef GSPC_NET_SERVER_BASE_CLIENT_HPP
#define GSPC_NET_SERVER_BASE_CLIENT_HPP

#include <map>
#include <string>
#include <vector>

#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <boost/asio.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include <fhg/util/thread/atomic.hpp>

#include <gspc/net/frame.hpp>
#include <gspc/net/frame_handler.hpp>
#include <gspc/net/parse/parser.hpp>

#include <gspc/net/client.hpp>
#include <gspc/net/client/response_fwd.hpp>
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
        base_client (boost::asio::io_service & io, endpoint_type const &);
        ~base_client ();

        int start ();
        int start (const boost::posix_time::time_duration);
        int stop ();

        int connect ();
        int connect (const boost::posix_time::time_duration);
        int disconnect ();

        bool is_connected () const;

        void set_frame_handler (frame_handler_t &);

        int send_raw (frame const &);

        int send (std::string const & dst, std::string const & body);
        int send_sync ( std::string const & dst
                      , std::string const & body
                      , const boost::posix_time::time_duration
                      );
        int send_sync ( frame const &rqst
                      , const boost::posix_time::time_duration
                      );
        int send_and_wait ( frame const &rqst
                          , frame &rply
                          , const boost::posix_time::time_duration
                          );

        int request ( std::string const &dst
                    , std::string const &body
                    , frame &rply
                    , const boost::posix_time::time_duration
                    );
        int request ( frame const &rqst, frame &rply
                    , const boost::posix_time::time_duration
                    );

        int subscribe ( std::string const &dest
                      , std::string const &id
                      );
        int unsubscribe (std::string const &id);

        int handle_frame (user_ptr, frame const &);
        int handle_error (user_ptr, boost::system::error_code const &);

        boost::system::error_code const & last_error_code () const;
        std::string const & get_private_queue () const;

        void set_timeout (size_t ms);
      private:
        bool try_notify_response (frame const & f);
        size_t abort_all_responses (boost::system::error_code const &);

        typedef boost::unique_lock<boost::shared_mutex> unique_lock;
        typedef boost::shared_lock<boost::shared_mutex> shared_lock;

        typedef boost::shared_ptr<response_t> response_ptr;
        typedef std::map<std::string, response_ptr> response_map_t;

        boost::asio::io_service        &m_io_service;
        endpoint_type                   m_endpoint;
        int                             m_state;
        connection_ptr_t                m_connection;

        frame_handler_t                *m_frame_handler;

        fhg::thread::atomic<size_t> m_message_id;

        mutable boost::shared_mutex      m_responses_mutex;
        response_map_t                   m_responses;
        boost::posix_time::time_duration m_timeout;

        boost::system::error_code   m_last_error_code;
        std::string m_priv_queue;
      };
    }
  }
}

#endif
