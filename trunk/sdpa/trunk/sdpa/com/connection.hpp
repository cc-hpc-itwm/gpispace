#ifndef SDPA_COM_CONNECTION_HPP
#define SDPA_COM_CONNECTION_HPP 1

#include <string>
#include <fhgcom/kvs/kvsc.hpp>
#include <boost/unordered_map.hpp>
#include <boost/unordered_set.hpp>
#include <boost/asio.hpp>

namespace sdpa
{
  namespace com
  {
    class connection;
    struct connection_listener
    {
      virtual void onConnect (connection *) = 0;
      virtual void onDisconnect (connection *, boost::system::error_code) = 0;
      virtual void onData (connection * c, std::string const &) = 0;
      virtual void onError (connection *, boost::system::error_code) = 0;
    };

    class connection
    {
    public:
      connection ( boost::asio::io_service & io_service
                 , std::string const & local_name
                 , std::string const & remote_name
                 );

      ~connection ();

      void remote (std::string const & s)
      {
        remote_ = s;
      }
      void local (std::string const & s)
      {
        local_ = s;
      }
      std::string const & remote () const
      {
        return remote_;
      }
      std::string const & local () const
      {
        return local_;
      }

      boost::asio::ip::tcp::socket & socket () { return socket_; }

      void send( const std::string & data
               , boost::posix_time::time_duration timeout
               );

    private:
      boost::asio::io_service & io_service_;
      boost::asio::ip::tcp::socket socket_;
      boost::asio::deadline_timer deadline_;

      std::string local_;
      std::string remote_;
    };

    class peer : public connection_listener
    {
    private:
      typedef boost::unordered_map<std::string, connection*> connection_map_t;
      typedef boost::unordered_set<connection*> connection_set_t;

    public:
      peer ( std::string const & name
           , std::string const & host
           , std::string const & port
           , connection_listener * listener
           , std::string const & kvs_host
           , std::string const & kvs_port = "2439"
           );

      ~peer ();

      void start ();
      void stop ();

      void send ( const std::string & to
                , const std::string & data
                , boost::posix_time::time_duration timeout
                );

      virtual void onConnect (connection *);
      virtual void onDisconnect (connection *, boost::system::error_code);
      virtual void onData (connection * c, std::string const &);
      virtual void onError (connection *, boost::system::error_code);
    private:
      std::string name_;
      std::string host_;
      std::string port_;

      fhg::com::kvs::client::kvsc kvs_;

      connection_listener * listener_;
      connection_map_t connections_;
      connection_set_t inbound_connections_;

      boost::asio::io_service io_service_;
      boost::asio::ip::tcp::acceptor acceptor_;
      boost::asio::deadline_timer deadline_;

      mutable boost::recursive_mutex mtx_;
    };
  }
}

#endif
