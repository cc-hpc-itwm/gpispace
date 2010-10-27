#include <fhglog/minimal.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/system/error_code.hpp>

#include "peer.hpp"
#include "kvs/kvsc.hpp"
#include <fhg/util/thread/event.hpp>

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
      , started_()
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
        boost::unique_lock<boost::recursive_mutex> lock(mutex_);

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
      // todo introduce timeout to event::wait()
      fhg::util::thread::event<int>::value_type ec;
      started_.wait (ec);
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

    void peer_t::send ( const std::string & to
                      , const std::string & data
                      )
    {
      typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
      async_op_t send_finished;
      async_send (to, data, boost::bind (&async_op_t::notify, &send_finished));

      boost::system::error_code ec;
      send_finished.wait (ec);
      if (ec)
      {
        throw boost::system::system_error (ec);
      }
    }

    void peer_t::async_send ( const message_t *m
                            , peer_t::handler_t completion_handler
                            )
    {
      assert (m);

      boost::unique_lock<boost::recursive_mutex> lock(mutex_);

      // TODO: io_service_.post (...);
      const p2p::address_t addr (m->header.dst);

      // TODO: short circuit loopback sends

      if (connections_.find(addr) == connections_.end())
      {
        // lookup location information
        std::string prefix ("p2p.peer");
        prefix += "." + boost::lexical_cast<std::string>(addr);
        kvs::values_type location (kvs::get_tree (prefix + "." + "location"));

        if (location.empty())
        {
          LOG(WARN, "could not lookup location information for " << addr);
          if (completion_handler)
          {
            try
            {
              using namespace boost::system;
              completion_handler
                (errc::make_error_code (errc::no_such_process));
            }
            catch (std::exception const & ex)
            {
              LOG(ERROR, "completion handler failed (ignored): " << ex.what());
            }
          }
          return;
        }

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
        to_send.message = *m;
        to_send.message.header.src = my_addr_;
        to_send.handler = completion_handler;
        cd.o_queue.push_back (to_send);

        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(h, p);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

        LOG(DEBUG, "initiating connection to " << addr << " at " << endpoint);
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
        to_send.message = *m;
        to_send.message.header.src = my_addr_;
        to_send.handler = completion_handler;
        cd.o_queue.push_back (to_send);

        io_service_.post (boost::bind (&self::start_sender, this, addr));
      }
    }

    void peer_t::send (const message_t *m)
    {
      assert (m);

      typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
      async_op_t send_finished;
      async_send (m, boost::bind (&async_op_t::notify, &send_finished));

      boost::system::error_code ec;
      send_finished.wait (ec);
      if (ec)
      {
        throw boost::system::system_error (ec);
      }
    }

    void peer_t::async_send ( const std::string & to
                            , const std::string & data
                            , peer_t::handler_t completion_handler
                            )
    {
      message_t m;
      m.assign (data.begin(), data.end());
      m.header.length = data.size();
      resolve_name (to, m.header.dst);
      async_send (&m, completion_handler);
    }

    void peer_t::recv (message_t *m)
    {
      assert (m);

      typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
      async_op_t recv_finished;
      async_recv (m, boost::bind (&async_op_t::notify, &recv_finished));

      boost::system::error_code ec;
      recv_finished.wait (ec);
      if (ec)
      {
        throw boost::system::system_error (ec);
      }
    }

    void peer_t::async_recv (message_t *m, peer_t::handler_t completion_handler)
    {
      assert (m);

      {
        boost::unique_lock<boost::recursive_mutex> lock(mutex_);
        // TODO: implement async receive on connection!
        if (m_pending.empty())
        {
          to_recv_t to_recv;
          to_recv.message = m;
          to_recv.handler = completion_handler;
          m_to_recv.push_back (to_recv);
          return;
        }
        else
        {
          const message_t *p = m_pending.front();
          m_pending.pop_front();
          *m = *p;
          delete p;
        }
      }

      using namespace boost::system;
      if (completion_handler)
        completion_handler(errc::make_error_code (errc::success));
    }

    void peer_t::resolve_name ( std::string const & name
                              , p2p::address_t & addr
                              )
    {
      addr = p2p::address_t (name);
    }

    void peer_t::resolve_addr ( p2p::address_t const & addr
                              , std::string & name
                              )
    {
      boost::unique_lock<boost::recursive_mutex> lock (mutex_);

      reverse_lookup_cache_t::const_iterator it (reverse_lookup_cache_.find (addr));
      if (it == reverse_lookup_cache_.end())
      {
        // lookup location information
        try
        {
          const std::string key
            ("p2p.peer." + boost::lexical_cast<std::string>(addr)+".name");
          name = kvs::get<std::string>(key);
          reverse_lookup_cache_[addr] = name;
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "could not resolve address (" << addr << ") to name: " << ex.what());
          throw std::runtime_error (std::string("could not resolve address: ") + ex.what());
        }
      }
      else
      {
        name = it->second;
      }
    }

    void peer_t::connection_established (const p2p::address_t a, boost::system::error_code const &ec)
    {
      if (! ec)
      {
        LOG(INFO, "connection to " << a << " established: " << ec);

        connection_data_t & cd = connections_.at (a);
        // send hello message
        to_send_t to_send;
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
        //     call handler for all o_queue elements
        //     call handler for all i_queue elements
      }
    }

    void peer_t::handle_send (const p2p::address_t a, boost::system::error_code const & ec)
    {
      assert (connections_.find (a) != connections_.end());
      connection_data_t & cd = connections_.at (a);
      assert (! cd.o_queue.empty());

      if (cd.o_queue.front().handler)
      {
        DLOG(TRACE, "message calling callback handler");
        cd.o_queue.front().handler (ec);
      }

      LOG_IF(WARN, ec, "message delivery to " << a << " failed: " << ec);

      cd.o_queue.pop_front();
      if (! ec)
      {
        if (! cd.o_queue.empty())
        {
          // TODO: wrap in strand...
          cd.connection->async_send ( &cd.o_queue.front().message
                                    , boost::bind (&self::handle_send, this, a, _1)
                                    );
        }
        else
        {
          cd.send_in_progress = false;
        }
      }
      else
      {
        cd.send_in_progress = false;
        // TODO: close connection
      }
    }

    void peer_t::start_sender (const p2p::address_t a)
    {
      connection_data_t & cd = connections_.at(a);
      if (cd.send_in_progress || cd.o_queue.empty())
        return;

      assert (! cd.o_queue.empty());

      cd.send_in_progress = true;
      cd.connection->async_send ( &cd.o_queue.front().message
                                , boost::bind (&self::handle_send, this, a, _1)
                                );
    }

    void peer_t::update_my_location ()
    {
      try
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

        started_.notify (0);
      }
      catch (...)
      {
        started_.notify (1);
        throw;
      }
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
          backlog_.erase (c);

          DLOG(INFO, "connection successfully established with " << m->header.src);

          connection_data_t & cd = connections_[m->header.src];
          if (cd.connection != 0)
          {
            // handle loopback connections correctly
            DLOG(TRACE, "connection already exists, it seems that i have initiated it");
            cd.loopback = c;
          }
          else
          {
            cd.connection = c;
          }
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
      assert (m);

      DLOG(TRACE, "got user message from: " << m->header.src);

      boost::unique_lock<boost::recursive_mutex> lock (mutex_);
      if (m_to_recv.empty())
      {
        // TODO: maybe add a flag to the message indicating whether it should be delivered
        // at all costs or not
        // if (m->header.flags & IMPORTANT)
        m_pending.push_back (m);
      }
      else
      {
        to_recv_t to_recv = m_to_recv.front();
        m_to_recv.pop_front();
        *to_recv.message = *m;
        delete m;

        using namespace boost::system;
        to_recv.handler
          (errc::make_error_code (errc::success));
      }
    }

    void peer_t::handle_error       (connection_t *, const boost::system::error_code & ec)
    {
      // TODO: WORK HERE
      LOG_IF(WARN, ec, "error on one of my connections, closing it...");
    }
  }
}
