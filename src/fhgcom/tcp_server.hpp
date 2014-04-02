#ifndef FHG_COM_TCP_SERVER_HPP
#define FHG_COM_TCP_SERVER_HPP 1

#include <fhgcom/session.hpp>
#include <fhgcom/session_manager.hpp>

#include <boost/asio.hpp>

namespace fhg
{
  namespace com
  {
    class tcp_server
    {
    public:
      typedef session_manager manager_t;

      tcp_server ( boost::asio::io_service&
                 , manager_t & manager
                 , const std::string & host
                 , const std::string & service
                 , const bool reuse_addr = true
                 );

      void stop ();

      unsigned short port () const;
    private:
      void accept ();
      void handle_accept ( boost::shared_ptr<session> session
                         , const boost::system::error_code & error
                         );

      boost::asio::io_service& _io_service;
      manager_t & manager_;

      boost::asio::ip::tcp::acceptor acceptor_;
    };
  }
}

#endif
