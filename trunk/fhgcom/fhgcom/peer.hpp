#ifndef FHG_COM_PEER_HPP
#define FHG_COM_PEER_HPP 1

#include <string>

#include <boost/unordered_map.hpp>

#include <fhgcom/connection.hpp>
#include <fhgcom/peer_info.hpp>

namespace fhg
{
  namespace com
  {
    class peer_t;
    struct peer_message_handler_t : public message_handler_t
    {
      peer_message_handler_t (peer_t * peer, std::string const & k)
        : peer_(peer)
        , key_(k)
      {}

      virtual void handle_error (const boost::system::error_code & ec);
      virtual void handle_recv  (const message_t * );
    private:
      peer_t * peer_;
      std::string key_;
    };

    /*!
      This class abstracts from an endpoint
     */
    class peer_t : private boost::noncopyable
    {
    private:
      typedef peer_t self;

    public:
      peer_t ( std::string const & name
             , host_t const & host
             , port_t const & port
             );

      std::string const & name () const { return name_; }
      std::string const & host () const { return host_; }
      std::string const & port () const { return port_; }

      void start ();
      void stop ();

      void send ( const std::string & to
                , const std::string & data
                );
      void recv_from ( std::string & from, std::string & data );
    private:
      void handle_accept (const boost::system::error_code &);

      std::string name_;
      std::string host_;
      std::string port_;

      boost::asio::io_service io_service_;
      boost::asio::ip::tcp::acceptor acceptor_;

      boost::unordered_map<std::string, connection_t *> connections_;
      connection_t * listen_;
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
