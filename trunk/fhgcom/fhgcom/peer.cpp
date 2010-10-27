#include <fhglog/minimal.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

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
      , my_addr_(p2p::address_t(name))
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
        try
        {
          std::string prefix ("p2p.peer");
          prefix += "." + boost::lexical_cast<std::string>(my_addr_);
          kvs::del (prefix);
        }
        catch (std::exception const & ex)
        {
          LOG(ERROR, "could not delete my information from the kvs: " << ex.what());
        }

        stop ();
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
      // lock(mutex)
      BOOST_FOREACH(connection_t * c, backlog_)
      {
        c->stop();
        delete c;
      }
      backlog_.clear();

      // TODO: call pending handlers and delete pending messages
      BOOST_FOREACH(connections_t::value_type cd, connections_)
      {
        cd.second.connection->stop();
        delete cd.second.connection;
      }
      connections_.clear();

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
        connection_data_t & cd = connections_[addr];
        cd.send_in_progress = false;
        cd.connection = new connection_t(io_service_, cookie_, this);

        to_send_t to_send;
        to_send.message.assign (data.begin(), data.end());
        to_send.message.header.src = my_addr_;
        to_send.message.header.dst = addr;
        to_send.message.header.length = data.size();
        cd.o_queue.push_back (to_send);

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
        connection_data_t & cd = connections_.at (addr);
        to_send_t to_send;
        to_send.message.assign (data.begin(), data.end());
        to_send.message.header.src = my_addr_;
        to_send.message.header.dst = addr;
        to_send.message.header.length = data.size();
        cd.o_queue.push_back (to_send);

        io_service_.post (boost::bind (&self::start_sender, this, addr));
      }

      //      throw std::runtime_error ("not implemented");
    }

    void peer_t::connection_established (const p2p::address_t a, boost::system::error_code const &ec)
    {
      if (! ec)
      {
        LOG(INFO, "connection to " << a << " established: " << ec);

        connection_data_t & cd = connections_.at (a);
        // send hello message
        to_send_t to_send;
        to_send.handler = 0;
        to_send.message.header.src = my_addr_;
        to_send.message.header.dst = a;
        to_send.message.header.type_of_msg = p2p::type_of_message_traits::HELLO_PACKET;

        cd.o_queue.push_front (to_send);
        start_sender (a);
      }
      else
      {
        LOG(WARN, "connection to " << a << " could not be established: " << ec);
        // TODO: remove connection data
      }
    }

    void peer_t::handle_send (const p2p::address_t a, boost::system::error_code const & ec)
    {
      connection_data_t & cd = connections_.at (a);
      to_send_t to_send = cd.o_queue.front ();
      cd.o_queue.pop_front();

      if (! ec)
      {
        DLOG(TRACE, "message successfully sent to " << a);

        if (! cd.o_queue.empty())
        {
          cd.connection->async_send ( &cd.o_queue.front().message
                                    , boost::bind (&self::handle_send, this, a, _1)
                                    );
        }
      }
      else
      {
        LOG(WARN, "could not send message to " << a << ": " << ec);
      }

      if (to_send.handler)
      {
        to_send.handler(ec);
      }
    }

    void peer_t::start_sender (const p2p::address_t a)
    {
      connection_data_t & cd = connections_.at(a);
      if (cd.send_in_progress)
        return;
      cd.connection->async_send ( &cd.o_queue.front().message
                                , boost::bind (&self::handle_send, this, a, _1)
                                );
      cd.send_in_progress = true;
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

      kvs::values_type values;
      values[prefix + "." + "location" + "." + "host"] =
        boost::lexical_cast<std::string>(endpoint.address());
      values[prefix + "." + "location" + "." + "port"] =
        boost::lexical_cast<std::string>(endpoint.port());
      values[prefix + "." + "name"] = name_;

      kvs::put (values);
    }

    void peer_t::handle_accept (const boost::system::error_code & ec)
    {
      DLOG(TRACE, "connection attempt from " << listen_->socket().remote_endpoint());
      // TODO: work here schedule timeout
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

      listen_ = new connection_t (io_service_, cookie_, this);
      acceptor_.async_accept( listen_->socket()
                            , boost::bind( &self::handle_accept
                                         , this
                                         , boost::asio::placeholders::error
                                         )
                            );
    }

    void peer_t::handle_system_data (connection_t *c, const message_t *m)
    {
      DLOG(TRACE, "got system message");
      switch (m->header.type_of_msg)
      {
      case p2p::type_of_message_traits::HELLO_PACKET:
        if (backlog_.find (c) == backlog_.end())
        {
          LOG(ERROR, "protocol error between " << my_addr_ << " and " << m->header.src << " closing connection");
          // TODO remove from other maps
        }
        else
        {
          DLOG(INFO, "connection successfully established with " << m->header.src);
          connection_data_t cd;
          cd.send_in_progress = false;
          cd.connection = c;
          connections_[m->header.src] = cd;
          backlog_.erase (c);
        }
        break;
      default:
        LOG(WARN, "cannot handle system message of type: " << std::hex << m->header.type_of_msg);
        break;
      }

      delete m;
    }
    void peer_t::handle_user_data   (connection_t *, const message_t *m)
    {
      DLOG(TRACE, "got user message");

    }
    void peer_t::handle_error       (connection_t *, const boost::system::error_code & ec)
    {
      // TODO: WORK HERE
      LOG_IF(WARN, ec, "error on one of my connections, closing it...");
    }
  }
}
