#include <fhglog/minimal.hpp>

#include "peer.hpp"

namespace fhg
{
  namespace com
  {
    peer_t::peer_t ( std::string const & name
                   , host_t const & host
                   , port_t const & port
                   )
      : name_(name)
      , host_(host)
      , port_(port)
      , io_service_()
      , acceptor_(io_service_)
      , connections_()
      , listen_(new connection_t (io_service_))
    {
      boost::asio::ip::tcp::resolver resolver(io_service_);
      boost::asio::ip::tcp::resolver::query query(host, port);
      boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
      acceptor_.open(endpoint.protocol());
      acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
      acceptor_.bind(endpoint);
      acceptor_.listen();
      acceptor_.async_accept( listen_->socket()
                            , boost::bind( &self::handle_accept
                                         , this
                                         , boost::asio::placeholders::error
                                         )
                            );
    }

    void peer_t::start()
    {

    }

    void peer_t::stop()
    {
    }

    void peer_t::handle_accept (const boost::system::error_code & ec)
    {
    }
  }
}
