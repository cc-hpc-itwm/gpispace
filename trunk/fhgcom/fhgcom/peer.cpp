#include <fhglog/minimal.hpp>

#include <boost/lexical_cast.hpp>

#include "peer.hpp"
#include "kvs/kvsc.hpp"

namespace fhg
{
  namespace com
  {
    peer_t::peer_t ( std::string const & name
                   , host_t const & host
                   , port_t const & port
                   , std::string const & cookie
                   )
      : name_(name)
      , host_(host)
      , port_(port)
      , cookie_(cookie)
      , io_service_()
      , acceptor_(io_service_)
      , connections_()
      , listen_(0)
    {
      if (name.find ('.') != std::string::npos)
      {
        throw std::runtime_error ("peer_t(): invalid argument: name must not contain '.'!");
      }

      boost::asio::ip::tcp::resolver resolver(io_service_);
      boost::asio::ip::tcp::resolver::query query(host, port);
      boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);
      acceptor_.open(endpoint.protocol());
      acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
      acceptor_.bind(endpoint);
      acceptor_.listen();

      io_service_.post (boost::bind (&self::update_my_location, this));

      accept_new ();
    }

    peer_t::~peer_t()
    {
      // stop and delete entries from backlog
      {
        std::string prefix ("p2p.peer");
        p2p::address_t addr (name_);
        prefix += "." + boost::lexical_cast<std::string>(p2p::address_t(name_));
        try
        {
          kvs::del (prefix);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not delete my information from the kvs: " << ex.what());
        }
      }
    }

    void peer_t::run ()
    {
      io_service_.run();
    }

    void peer_t::start()
    {
    }

    void peer_t::stop()
    {
      io_service_.stop();
    }

    void peer_t::async_send ( const std::string & to
                            , const std::string & data
                            , peer_t::handler_t completion_handler
                            )
    {
      // TODO: io_service_.post (...);
      p2p::address_t addr (to);

      // create message

      if (connections_.find(addr) == connections_.end())
      {
        // lookup location information
        std::string prefix ("p2p.peer");
        prefix += "." + boost::lexical_cast<std::string>(addr);
        kvs::values_type location (kvs::get_tree (prefix + "." + "location"));

        assert (! location.empty());

        host_t h (location.at(prefix + ".location.host"));
        port_t p (location.at(prefix + ".location.port"));

        // store message in out queue
        //    connect_handler -> sends messages from out queue
        //    error_handler -> clears messages from out queue
        // async_connect (...);
        connection_data_t cd;
        cd.connection = new connection_t(io_service_, this);
        connections_[addr] = cd;

        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(h, p);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

        LOG(DEBUG, "initiating connection to " << to << " at " << endpoint);
        cd.connection->socket().async_connect( endpoint
                                             , boost::bind ( &self::connection_established
                                                           , this
                                                           , addr
                                                           , boost::asio::placeholders::error
                                                           )
                                             );
      }
      else
      {
      }

      //      throw std::runtime_error ("not implemented");
    }

    void peer_t::connection_established (const p2p::address_t a, boost::system::error_code const &ec)
    {
      LOG(INFO, "connection to " << a << " established: " << ec);
    }

    void peer_t::async_recv ( std::string & from
                            , std::string & data
                            , peer_t::handler_t completion_handler
                            )
    {
      //      throw std::runtime_error ("not implemented");
    }

    void peer_t::update_my_location ()
    {
      boost::asio::ip::tcp::endpoint endpoint = acceptor_.local_endpoint();

      std::string prefix ("p2p.peer");
      prefix += "." + boost::lexical_cast<std::string>(p2p::address_t(name_));

      kvs::put ( prefix + "." + "location" + "." + "host"
               , boost::lexical_cast<std::string>(endpoint.address())
               );
      kvs::put ( prefix + "." + "location" + "." + "port"
               , boost::lexical_cast<std::string>(endpoint.port())
               );
      kvs::put ( prefix + "." + "name"
               , name_
               );
    }

    void peer_t::handle_accept (const boost::system::error_code & ec)
    {
      DLOG(TRACE, "connection attempt");
      backlog_.insert (listen_);

      // the connection will  call us back when it got the  hello packet or will
      // timeout
      listen_->start ();
      listen_ = 0;

      // create new connection to accept on
      accept_new ();
    }

    void peer_t::accept_new ()
    {
      assert (0 == listen_);

      listen_ = new connection_t (io_service_, this);
      listen_->set_callback_handler (this);
      acceptor_.async_accept( listen_->socket()
                            , boost::bind( &self::handle_accept
                                         , this
                                         , boost::asio::placeholders::error
                                         )
                            );
    }
  }
}
