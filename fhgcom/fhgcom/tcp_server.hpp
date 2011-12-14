#ifndef FHG_COM_TCP_SERVER_HPP
#define FHG_COM_TCP_SERVER_HPP 1

#include <boost/asio.hpp>

#include <fhgcom/session.hpp>
#include <fhgcom/session_manager.hpp>

#include <fhgcom/io_service_pool.hpp>

namespace fhg
{
  namespace com
  {
    class tcp_server
    {
    public:
      typedef session_manager<session> manager_t;
      typedef manager_t::session_type session_type;
      typedef manager_t::session_ptr  session_ptr;

      tcp_server ( io_service_pool & pool
                 , manager_t & manager
                 , const std::string & host
                 , const std::string & service
                 , const bool reuse_addr = true
                 );

      void start (void);
      void start ( const std::string & host
                 , const std::string & service
                 , const bool reuse_addr = true
                 );
      void stop ();

      unsigned short port () const;
    private:
      bool try_start ( boost::asio::ip::tcp::endpoint ep
                     , boost::system::error_code & ec
                     , bool reuse_addr = true
                     );
      void accept ();
      void handle_accept ( session_ptr session
                         , const boost::system::error_code & error
                         );

      io_service_pool &  service_pool_;
      manager_t & manager_;

      std::string host_;
      std::string service_;
      bool reuse_addr_;

      boost::asio::ip::tcp::acceptor acceptor_;
    };
  }
}

#endif
