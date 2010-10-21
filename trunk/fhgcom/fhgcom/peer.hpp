#ifndef FHG_COM_PEER_HPP
#define FHG_COM_PEER_HPP 1

#include <string>

#include <boost/system/error_code.hpp>

#include <boost/unordered_set.hpp>
#include <boost/unordered_map.hpp>

#include <fhgcom/header.hpp>
#include <fhgcom/connection.hpp>
#include <fhgcom/peer_info.hpp>

namespace fhg
{
  namespace com
  {
    /*!
      This class abstracts from an endpoint
     */
    class peer_t : public connection_t::handler_t
                 , private boost::noncopyable
    {
    private:
      typedef peer_t self;

    public:
      typedef void (*handler_t)(boost::system::error_code const &);

      peer_t ( std::string const & name
             , host_t const & host
             , port_t const & port
             , std::string const & cookie = ""
             );

      virtual ~peer_t ();

      std::string const & name () const { return name_; }
      std::string const & host () const { return host_; }
      std::string const & port () const { return port_; }

      void start ();
      void stop ();
      void run ();

      void async_send ( const std::string & to
                      , const std::string & data
                      , handler_t h = 0
                      );
      void async_recv ( std::string & from
                      , std::string & data
                      , handler_t h = 0
                      );
    protected:
      void handle_system_data (connection_t *, const message_t *m) { delete m; }
      void handle_user_data   (connection_t *, const message_t *m) { delete m; }
      void handle_error       (connection_t *, const boost::system::error_code & error) {}

    private:
      struct to_send_t
      {
        message_t *message;
        handler_t  handler;
      };

      struct to_recv_t
      {
        message_t *message;
        handler_t  handler;
        // semaphore...
        // timeout
      };

      struct connection_data_t
      {
        connection_t *connection;
        std::vector<to_send_t> i_queue;
        std::vector<to_recv_t> o_queue;
      };

      void accept_new ();
      void handle_accept (const boost::system::error_code &);
      void update_my_location ();
      void connection_established (const p2p::address_t, boost::system::error_code const &);

      std::string name_;
      std::string host_;
      std::string port_;
      std::string cookie_;

      boost::asio::io_service io_service_;
      boost::asio::ip::tcp::acceptor acceptor_;

      boost::unordered_map<p2p::address_t, connection_data_t> connections_;
      boost::unordered_set<connection_t *> backlog_;
      connection_t * listen_;

      std::vector<message_t *> incoming_;
    };

    inline
    std::ostream & operator << (std::ostream & os, peer_t const & p)
    {
      return os << p.name() << "@"
                << "[" << p.host() << "]:" << p.port();
    }
  }
}

#endif
