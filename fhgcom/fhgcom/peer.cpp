#include <fhglog/minimal.hpp>
#include <fhg/assert.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/make_shared.hpp>

#include <boost/lambda/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

using boost::lambda::bind;
using boost::lambda::var;
//using boost::lambda::_1;

#include "peer.hpp"
#include "kvs/kvsc.hpp"
#include <fhg/util/thread/event.hpp>
#include <fhg/util/random.hpp>

namespace fhg
{
  namespace com
  {
    static void dummy_handler (boost::system::error_code const &)
    {}

    static void default_kvs_error_handler (boost::system::error_code const &)
    {
      MLOG (ERROR, "could not contact KVS...");
    }

    peer_t::peer_t ( std::string const & name
                   , host_t const & host
                   , port_t const & port
                   , std::string const & cookie
                   )
      : stopped_(true)
      , name_(name)
      , host_(host)
      , port_(port)
      , cookie_(cookie)
      , my_addr_(p2p::address_t(name))
      , started_()
      , io_service_()
      , io_service_work_(new boost::asio::io_service::work(io_service_))
      , acceptor_(io_service_)
      , m_renew_kvs_entries_timer (io_service_)
      , connections_()
      , m_kvs_error_handler (default_kvs_error_handler)
    {
      if (name.find ('.') != std::string::npos)
      {
        throw std::runtime_error ("peer_t(): invalid argument: name must not contain '.'!");
      }
      if (name.find (' ') != std::string::npos)
      {
        throw std::runtime_error ("peer_t(): invalid argument: name must not contain ' '!");
      }
    }

    peer_t::~peer_t()
    {
      try
      {
        stop();
      }
      catch (std::exception const & ex)
      {
        LOG(ERROR, "exception during destructor of peer " << name() << ": " << ex.what());
      }
    }

    std::string peer_t::hostname()
    {
      return boost::asio::ip::host_name();
    }

    peer_t::handler_t peer_t::set_kvs_error_handler (handler_t h)
    {
      handler_t old = m_kvs_error_handler;
      m_kvs_error_handler = h;
      return old;
    }

    peer_t::handler_t peer_t::get_kvs_error_handler () const
    {
      return m_kvs_error_handler;
    }

    void peer_t::run ()
    {
      if (m_peer_thread)
        return;
      m_peer_thread = boost::make_shared<boost::thread>
        (boost::bind ( &boost::asio::io_service::run
                     , &io_service_
                     )
        );
      m_peer_thread->join ();
    }

    void peer_t::start()
    {
      {
        lock_type lock(mutex_);

        if (! stopped_)
          return;

        boost::asio::ip::tcp::resolver resolver(io_service_);
        boost::asio::ip::tcp::resolver::query query(host_, port_);
        boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

        const bool prefer_ipv6 = false;

        if (host_ == "*")
        {
          if (prefer_ipv6)
          {
            endpoint.address(boost::asio::ip::address_v6::any());
          }
          else
          {
            endpoint.address(boost::asio::ip::address_v4::any());
          }
        }

        acceptor_.open(endpoint.protocol());
        acceptor_.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        acceptor_.bind(endpoint);
        acceptor_.listen();

        accept_new ();

        io_service_.post (boost::bind ( &self::update_my_location
                                      , this
                                      )
                         );
      }

      boost::system::error_code ec;
      // todo introduce timeout to event::wait()
      started_.wait (ec);

      if (ec)
      {
        stop();
        throw boost::system::system_error (ec);
      }

      {
        lock_type lock(mutex_);
        stopped_ = false;
      }
    }

    void peer_t::stop()
    {
      io_service_.stop();

      DLOG(TRACE, "stopping peer " << name());

      lock_type lock(mutex_);

      listen_.reset ();

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
      }

      backlog_.clear ();

      // TODO: call pending handlers and delete pending messages
      while (! connections_.empty ())
      {
        connection_data_t & cd = connections_.begin()->second;

        if (cd.connection)
        {
          boost::system::error_code ignore;
          cd.connection->socket ().cancel (ignore);
        }

        while (! cd.o_queue.empty())
        {
          to_send_t & to_send = cd.o_queue.front();
          using namespace boost::system;
          to_send.handler (errc::make_error_code(errc::operation_canceled));
          cd.o_queue.pop_front();
        }

        connections_.erase (connections_.begin());
      }

      // remove pending
      while (! m_pending.empty())
      {
        delete m_pending.front();
        m_pending.pop_front();
      }

      while (! m_to_recv.empty())
      {
        to_recv_t to_recv = m_to_recv.front();
        m_to_recv.pop_front();
        using namespace boost::system;
        to_recv.handler (errc::make_error_code(errc::operation_canceled));
      }

      stopped_ = true;
    }

    void peer_t::send ( const std::string & to
                      , const std::string & data
                      )
    {
      typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
      async_op_t send_finished;
      async_send (to, data, boost::bind (&async_op_t::notify, &send_finished, _1));

      boost::system::error_code ec;
      send_finished.wait (ec);
      DLOG(TRACE, "send finished with ec: " << ec);
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
      assert (completion_handler);

      lock_type lock(mutex_);

      // TODO: io_service_.post (...);
      const p2p::address_t addr (m->header.dst);

      // TODO: short circuit loopback sends

      if (connections_.find(addr) == connections_.end())
      {
        DLOG(TRACE, "initiating connection to " << m->header.dst);

        // lookup location information
        std::string prefix ("p2p.peer");
        prefix += "." + boost::lexical_cast<std::string>(addr);
        kvs::values_type peer_info (kvs::get_tree (prefix));

        if (peer_info.empty())
        {
          try
          {
            using namespace boost::system;
            completion_handler(errc::make_error_code (errc::no_such_process));
          }
          catch (std::exception const & ex)
          {
            LOG (ERROR, "completion handler failed (ignored): " << ex.what());
          }
          return;
        }

        host_t h (peer_info.at(prefix + ".location.host"));
        port_t p (peer_info.at(prefix + ".location.port"));
        std::string n (peer_info.at(prefix + ".name"));
        reverse_lookup_cache_[addr] = n;

        DLOG( TRACE
            , "corresponding connection data:"
            << " name=" << n
            << " host=" << std::string(h)
            << " port=" << std::string(p)
            );

        // store message in out queue
        //    connect_handler -> sends messages from out queue
        //    error_handler -> clears messages from out queue
        // async_connect (...);
        connection_data_t & cd = connections_[addr];
        cd.send_in_progress = false;
        cd.name = n;

        to_send_t to_send;
        to_send.message = *m;
        to_send.message.header.length = to_send.message.data.size();
        to_send.message.header.src = my_addr_;
        to_send.handler = completion_handler;
        cd.o_queue.push_back (to_send);

        cd.connection =
          connection_t::ptr_t(new connection_t( io_service_
                                              , cookie_
                                              , this
                                              )
                             );
        cd.connection->local_address (my_addr_);
        cd.connection->remote_address (addr);

        namespace bai = boost::asio::ip;

        bai::tcp::resolver resolver(io_service_);
        bai::tcp::resolver::query query(h, p);
        bai::tcp::resolver::iterator iter =
          bai::tcp::resolver(io_service_).resolve(query);

        boost::system::error_code ec;

        for (; iter != bai::tcp::resolver::iterator(); ++iter)
        {
          cd.connection->socket().close();

          ec = boost::asio::error::would_block;

          cd.connection->socket().connect( iter->endpoint(), ec);

          if (!ec && cd.connection->socket().is_open())
          {
            break;
          }
        }

        connection_established (addr, ec);
      }
      else
      {
        connection_data_t & cd = connections_.at (addr);
        to_send_t to_send;
        to_send.message = *m;
        to_send.message.header.length = to_send.message.data.size();
        to_send.message.header.src = my_addr_;
        to_send.handler = completion_handler;
        cd.o_queue.push_back (to_send);

        io_service_.post (boost::bind ( &self::start_sender
                                      , this
                                      , addr
                                      )
                         );
      }
    }

    void peer_t::send (const message_t *m)
    {
      assert (m);

      typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
      async_op_t send_finished;

      async_send (m, boost::bind (&async_op_t::notify, &send_finished, _1));

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
      async_recv (m, boost::bind (&async_op_t::notify, &recv_finished, _1));

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
      assert (completion_handler);

      {
        lock_type lock(mutex_);
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
      completion_handler(errc::make_error_code (errc::success));
    }

    std::string peer_t::resolve_addr (p2p::address_t const &addr)
    {
      lock_type lock (mutex_);

      reverse_lookup_cache_t::const_iterator it (reverse_lookup_cache_.find (addr));
      if (it == reverse_lookup_cache_.end())
      {
        // lookup location information
        try
        {
          const std::string key
            ("p2p.peer." + boost::lexical_cast<std::string>(addr)+".name");
          std::string name = kvs::get<std::string>(key);
          reverse_lookup_cache_[addr] = name;
          return name;
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "could not resolve address (" << addr << ") to name: " << ex.what());
          throw std::runtime_error (std::string("could not resolve address: ") + ex.what());
        }
      }
      else
      {
        return it->second;
      }
    }

    std::string peer_t::resolve ( p2p::address_t const & addr
                                , std::string const & dflt
                                )
    {
      try
      {
        return resolve_addr (addr);
      }
      catch (...)
      {
        return dflt;
      }
    }

    p2p::address_t peer_t::resolve ( std::string const & name )
    {
      return p2p::address_t (name);
    }

    p2p::address_t peer_t::resolve_name (std::string const &name)
    {
      return p2p::address_t (name);
    }

    void peer_t::resolve_name ( std::string const & name
                              , p2p::address_t & addr
                              )
    {
      addr = resolve_name (name);
    }

    void peer_t::resolve_addr ( p2p::address_t const & addr
                              , std::string & name
                              )
    {
      name = resolve_addr (addr);
    }

    void peer_t::connection_established (const p2p::address_t a, boost::system::error_code const &ec)
    {
      lock_type lock (mutex_);

      if (! ec)
      {
        connection_data_t & cd = connections_.find (a)->second;

        LOG( TRACE
             , my_addr_ << " (" << name_ << ")"
             << " connected to "
             << a << " (" << cd.name << ")"
             << " @ " << cd.connection->socket().remote_endpoint();
             );

        {
          DLOG(TRACE, "setting socket option 'keep-alive' to 'true'");
          boost::asio::socket_base::keep_alive o(true);
          cd.connection->set_option (o);
        }

        // send hello message
        to_send_t to_send;
        to_send.handler = dummy_handler;
        to_send.message.header.src = my_addr_;
        to_send.message.header.dst = a;
        to_send.message.header.type_of_msg = p2p::type_of_message_traits::HELLO_PACKET;
        to_send.message.header.length = name_.size();
        to_send.message.assign (name_.begin(), name_.end());

        cd.connection->start ();
        cd.o_queue.push_front (to_send);
        start_sender (a);
      }
      else
      {
        DLOG(WARN, "connection to " << a << " could not be established: " << ec);

        if (connections_.find (a) != connections_.end())
        {
          connection_data_t & cd = connections_.find (a)->second;
          handle_error (cd.connection, ec);
        }
      }
    }

    void peer_t::handle_send (const p2p::address_t a, boost::system::error_code const & ec)
    {
      lock_type lock (mutex_);

      if (connections_.find (a) == connections_.end())
      {
        LOG_IF( WARN
              , ec
              , "could not send message to " << a
              << " connection already closed: "
              << ec << " msg: " << ec.message ()
              );
        return;
      }

      connection_data_t & cd = connections_.at (a);
      if (cd.o_queue.empty ())
      {
        MLOG_IF ( WARN
                , cd.send_in_progress
                , "inconsistent output queue: " << ec << " msg: " << ec.message ()
                );
        return;
      }

      cd.o_queue.front().handler (ec);
      cd.o_queue.pop_front();

      LOG_IF(WARN, ec, "message delivery to " << a << " failed: " << ec);

      if (! ec)
      {
        if (! cd.o_queue.empty())
        {
          // TODO: wrap in strand...
          cd.connection->async_send ( &cd.o_queue.front().message
                                    , boost::bind ( &self::handle_send
                                                  , this
                                                  , a
                                                  , _1
                                                  )
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
      lock_type lock (mutex_);

      try
      {
        connection_data_t & cd = connections_.at(a);
        if (cd.send_in_progress || cd.o_queue.empty())
          return;

        assert (! cd.o_queue.empty());

        cd.send_in_progress = true;
        cd.connection->async_send ( &cd.o_queue.front().message
                                  , boost::bind ( &self::handle_send
                                                , this
                                                , a
                                                , _1
                                                )
                                  );
      }
      catch (std::out_of_range const &)
      {
        // ignore, connection has been closed before we could start it
      }
      catch (std::exception const & ex)
      {
        LOG (ERROR, "could not start sender to " << a << ": " << ex.what ());
      }
    }

    void peer_t::renew_kvs_entries ()
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
      values[prefix + "." + "pid"] =
        boost::lexical_cast<std::string>(getpid ());

      if (  (endpoint.address() == boost::asio::ip::address_v4::any())
         || (endpoint.address() == boost::asio::ip::address_v6::any())
         )
      {
        const std::string h(boost::asio::ip::host_name());
        DMLOG( TRACE
            , "endpoint is any address, changing registration host to: " << h
            );
        values[prefix + "." + "location" + "." + "host"] = h;
      }

      try
      {
        kvs::timed_put (values, 2 * 60 * 1000u);
      }
      catch (std::exception const &ex)
      {
        using namespace boost::system;

        m_kvs_error_handler (errc::make_error_code (errc::connection_refused));
      }

      m_renew_kvs_entries_timer.expires_from_now
        (boost::posix_time::seconds (fhg::util::random::rand_in (60, 90)));

      m_renew_kvs_entries_timer.async_wait
        (boost::bind (&peer_t::renew_kvs_entries, this));
    }

    void peer_t::update_my_location ()
    {
      try
      {
        renew_kvs_entries ();

        started_.notify (boost::system::error_code());
      }
      catch (boost::system::system_error const &bse)
      {
        LOG(ERROR, "could not update my location: " << bse.what());
        started_.notify (bse.code());
      }
      catch (std::exception const &ex)
      {
        LOG(ERROR, "could not update my location: " << ex.what());
        started_.notify
          (boost::system::errc::make_error_code
          (boost::system::errc::state_not_recoverable)
          );
      }
    }

    void peer_t::handle_accept (const boost::system::error_code & ec)
    {
      lock_type lock (mutex_);

      if (ec)
      {
        LOG (WARN, "accept() failed: " << ec << " := " << ec.message ());
      }
      else
      {
        if (listen_)
        {
          DLOG(TRACE, "connection attempt from " << listen_->socket().remote_endpoint());
          // TODO: work here schedule timeout
          backlog_.insert (listen_);

          // the connection will  call us back when it got the  hello packet or will
          // timeout
          listen_->set_option
            (boost::asio::socket_base::keep_alive (true));
          listen_->start ();
          listen_.reset ();
        }

        accept_new ();
      }
    }

    void peer_t::accept_new ()
    {
      assert (! listen_);

      listen_ = connection_t::ptr_t (new connection_t
                                    ( io_service_
                                    , cookie_
                                    , this
                                    ));
      listen_->local_address(my_addr_);
      listen_->remote_address(p2p::address_t());
      acceptor_.async_accept( listen_->socket()
                            , boost::bind( &self::handle_accept
                                         , this
                                         , boost::asio::placeholders::error
                                         )
                            );
    }

    void peer_t::handle_system_data (connection_t::ptr_t c, const message_t *m)
    {
      lock_type lock (mutex_);

      DLOG(TRACE, "got system message");
      switch (m->header.type_of_msg)
      {
      case p2p::type_of_message_traits::HELLO_PACKET:
        if (backlog_.find (c) == backlog_.end())
        {
          LOG(ERROR, "protocol error between " << my_addr_ << " and " << m->header.src << " closing connection");
          try
          {
            c->stop();
          }
          catch (std::exception const & ex)
          {
            LOG(WARN, "could not stop connection: " << ex.what());
          }
        }
        else
        {
          backlog_.erase (c);

          c->remote_address (m->header.src);
          c->local_address (m->header.dst);

          const std::string remote_name
            (m->buf(), m->header.length);
          reverse_lookup_cache_[m->header.src] = remote_name;

          DLOG( DEBUG
              , "connection between "
              << my_addr_ << " (" << name_ << ")"
              << " and "
              << m->header.src << " (" << remote_name << ")"
              << " successfully established"
              );

          connection_data_t & cd = connections_[m->header.src];
          cd.name = remote_name;
          if (cd.connection)
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

    void peer_t::handle_user_data   (connection_t::ptr_t c, const message_t *m)
    {
      assert (m);

      DLOG(TRACE, "got user message from: " << m->header.src);

      lock_type lock (mutex_);
      {
        if (m_to_recv.empty())
        {
          // TODO: maybe add a flag to the message indicating whether it should be delivered
          // at all costs or not
          // if (m->header.flags & IMPORTANT)
          m_pending.push_back (m);
          return;
        }
        else
        {
          to_recv_t to_recv = m_to_recv.front();
          m_to_recv.pop_front();
          *to_recv.message = *m;
          delete m;

          using namespace boost::system;

          lock.unlock ();
          to_recv.handler(errc::make_error_code (errc::success));
          lock.lock ();
        }
      }
    }

    void peer_t::handle_error (connection_t::ptr_t c, const boost::system::error_code & ec)
    {
      assert ( c != NULL );

      lock_type lock (mutex_);

      if (connections_.find (c->remote_address()) != connections_.end())
      {
        connection_data_t & cd = connections_[c->remote_address()];

        boost::system::error_code ignore;
        c->socket ().cancel (ignore);
        c->socket ().close (ignore);

        // deactivate asynchronous sender
        cd.send_in_progress = false;

        DLOG_IF( WARN
               , ec && (ec.value() != boost::asio::error::eof)
               , "error on connection to " << cd.name << " - closing it: cat=" << ec.category().name() << " val=" << ec.value() << " txt=" << ec.message()
               );

        while (! cd.o_queue.empty())
        {
          to_send_t to_send = cd.o_queue.front();
          cd.o_queue.pop_front();

          using namespace boost::system;

          lock.unlock ();
          to_send.handler (ec);
          lock.lock ();
        }

        // the handler might async recv again...
        std::list<to_recv_t> tmp (m_to_recv);
        m_to_recv.clear();

        while (! tmp.empty())
        {
          to_recv_t to_recv = tmp.front();
          tmp.pop_front();

          using namespace boost::system;
          to_recv.message->header.src = c->remote_address();
          to_recv.message->header.dst = c->local_address();

          lock.unlock ();
          to_recv.handler (ec);
          lock.lock ();
        }

        connections_.erase(c->remote_address());
      }
      else
      {
        if (backlog_.find (c) != backlog_.end ())
        {
          backlog_.erase (c);
        }
        else
        {
          /*
          LOG_IF ( ERROR
                 , (c->remote_address () != p2p::address_t() && (ec.value() != boost::asio::error::eof))
                 , "error on a connection i don't know anything about, remote: " << c->remote_address()
                 << " error: " << ec << " msg: " << ec.message ()
                 );
          */
          c->stop ();
          c.reset ();
        }
      }
    }
  }
}
