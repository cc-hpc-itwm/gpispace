#include "tcp_server.hpp"
#include <boost/bind.hpp>

using namespace fhg::com;
using namespace boost::asio::ip;

tcp_server::tcp_server ( io_service_pool & pool
                       , tcp_server::manager_t & manager
                       , const std::string & host
                       , const std::string & service
                       , const bool reuse_addr
                       )
  : service_pool_ (pool)
  , manager_(manager)
  , host_(host)
  , service_(service)
  , reuse_addr_(reuse_addr)
  , acceptor_(pool.get_io_service())
{
}

void tcp_server::stop ()
{
  acceptor_.close();
}

unsigned short tcp_server::port () const
{
  return acceptor_.local_endpoint().port();
}

void tcp_server::start ()
{
  start (host_, service_, reuse_addr_);
}

void tcp_server::start ( const std::string & host
                       , const std::string & service
                       , const bool reuse_addr
                       )
{
  acceptor_.close ();

  boost::system::error_code ec;

  boost::asio::ip::tcp::resolver resolver (service_pool_.get_io_service());
  boost::asio::ip::tcp::resolver::query query(host, service);
  boost::asio::ip::tcp::resolver::iterator
    endpoint_iterator(resolver.resolve (query));

  while (endpoint_iterator != boost::asio::ip::tcp::resolver::iterator())
  {
    if   (try_start(*endpoint_iterator, ec, reuse_addr)) break;
    else ++endpoint_iterator;
  }

  // throw remaining error if any
  boost::asio::detail::throw_error (ec);
}

bool tcp_server::try_start ( boost::asio::ip::tcp::endpoint endpoint
                           , boost::system::error_code & ec
                           , bool reuse_addr
                           )
{
  DLOG(TRACE, "trying to bind to " << endpoint);
  acceptor_.close ();
  acceptor_.open (endpoint.protocol(), ec);

  if (ec) return false;

  if (reuse_addr)
  {
    acceptor_.set_option (tcp::acceptor::reuse_address(true), ec);
    if (ec) return false;
  }

  acceptor_.bind (endpoint, ec);
  if (! ec)
  {
    acceptor_.listen (tcp::acceptor::max_connections, ec);
    if (! ec)
    {
      LOG(DEBUG, "successfully bound to: " << acceptor_.local_endpoint());
      accept ();
    }
  }

  if (ec)
  {
    LOG(WARN, "could not bind to " << endpoint << " = " << ec.message() << ": " << ec);
    acceptor_.close();
    return false;
  }

  return true;
}

void tcp_server::accept ()
{
  session_ptr new_session( new session( service_pool_.get_io_service()
                                      , manager_)
                         );
  acceptor_.async_accept
    ( new_session->socket()
    , boost::bind ( &tcp_server::handle_accept
                  , this
                  , new_session
                  , boost::asio::placeholders::error
                  )
    );
}

void tcp_server::handle_accept ( tcp_server::session_ptr session
                               , const boost::system::error_code & error
                               )
{
  if (!error)
  {
    session->start();
    accept();
  }
}
