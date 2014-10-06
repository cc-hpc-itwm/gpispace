#include <fhgcom/peer.hpp>

#include <fhglog/LogMacros.hpp>

#include <fhg/assert.hpp>
#include <fhg/util/thread/event.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/make_shared.hpp>
#include <boost/system/error_code.hpp>
#include <boost/system/system_error.hpp>

#include <cstdlib>
#include <functional>

namespace fhg
{
  namespace com
  {
    peer_t::peer_t ( std::string const & name
                   , host_t const & host
                   , port_t const & port
                   , kvs::kvsc_ptr_t kvs_client
                   , handler_t handler
                   )
      : stopped_(true)
      , stopping_ (false)
      , name_(name)
      , host_(host)
      , port_(port)
      , my_addr_(p2p::address_t(name))
      , started_()
      , _kvs_client (kvs_client)
      , io_service_()
      , io_service_work_(new boost::asio::io_service::work(io_service_))
      , acceptor_(io_service_)
      , m_renew_kvs_entries_timer (io_service_)
      , connections_()
      , m_kvs_error_handler (handler)
    {
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

    void peer_t::run ()
    {
      if (m_peer_thread)
        return;
      m_peer_thread = boost::make_shared<boost::thread>
        ([this] { io_service_.run(); });
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

        io_service_.post (std::bind (&peer_t::update_my_location, this));
      }

      // todo introduce timeout to event::wait()
      const boost::system::error_code ec (started_.wait());

      if (ec)
      {
        stop();
        throw boost::system::system_error (ec);
      }

      {
        lock_type lock(mutex_);
        stopped_ = false;
        stopping_ = false;
      }
    }

    void peer_t::stop()
    {
      lock_type lock(mutex_);

      stopping_ = true;

      acceptor_.close ();

      if (listen_)
      {
        listen_->socket ().close ();
      }

      {
        try
        {
          std::string prefix ("p2p.peer");
          prefix += "." + p2p::to_string (my_addr_);
          _kvs_client->del (prefix);
        }
        catch (std::exception const & ex)
        {
        }
      }

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
        delete m_pending.front().first;
        m_pending.pop_front();
      }

      while (! m_to_recv.empty())
      {
        to_recv_t to_recv = m_to_recv.front();
        m_to_recv.pop_front();
        using namespace boost::system;
        to_recv.handler (errc::make_error_code(errc::operation_canceled), boost::none);
      }

      backlog_.clear ();

      io_service_.stop();

      stopping_ = false;
      stopped_ = true;
    }

    void peer_t::send ( const std::string & to
                      , const std::string & data
                      )
    {
      typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
      async_op_t send_finished;
      async_send
        ( to, data
        , std::bind (&async_op_t::notify, &send_finished, std::placeholders::_1)
        );

      const boost::system::error_code ec (send_finished.wait());
      if (ec)
      {
        throw boost::system::system_error (ec);
      }
    }

    void peer_t::async_send ( const message_t *m
                            , peer_t::handler_t completion_handler
                            )
    {
      fhg_assert (m);
      fhg_assert (completion_handler);

      lock_type lock(mutex_);

      if (stopping_)
      {
        using namespace boost::system;
        completion_handler (errc::make_error_code (errc::network_down));
        return;
      }

      // TODO: io_service_.post (...);
      const p2p::address_t addr (m->header.dst);

      if (connections_.find(addr) == connections_.end())
      {
        // lookup location information
        std::string prefix ("p2p.peer");
        prefix += "." + p2p::to_string (addr);
        kvs::values_type peer_info (_kvs_client->get (prefix));

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

        cd.connection = connection_t::ptr_t
          ( new connection_t
            ( io_service_
            , std::bind (&peer_t::handle_hello_message, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&peer_t::handle_user_data, this, std::placeholders::_1, std::placeholders::_2)
            , std::bind (&peer_t::handle_error, this, std::placeholders::_1, std::placeholders::_2)
            )
          );
        cd.connection->local_address (my_addr_);
        cd.connection->remote_address (addr, cd.name);

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

        if (cd.o_queue.size () == 1)
          start_sender (addr);
      }
    }

    void peer_t::send (const message_t *m)
    {
      fhg_assert (m);

      typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
      async_op_t send_finished;

      async_send
        ( m
        , std::bind (&async_op_t::notify, &send_finished, std::placeholders::_1)
        );

      const boost::system::error_code ec (send_finished.wait());
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
      m.header.dst = p2p::address_t (to);
      async_send (&m, completion_handler);
    }

    void peer_t::recv (message_t *m)
    {
      fhg_assert (m);

      typedef fhg::util::thread::event<boost::system::error_code> async_op_t;
      async_op_t recv_finished;
      async_recv
        ( m
        , std::bind (&async_op_t::notify, &recv_finished, std::placeholders::_1)
        );

      const boost::system::error_code ec (recv_finished.wait());
      if (ec)
      {
        throw boost::system::system_error (ec);
      }
    }

    void peer_t::async_recv
      ( message_t *m
      , std::function<void ( boost::system::error_code
                           , boost::optional<std::string> source_name
                           )
                     > completion_handler
      )
    {
      fhg_assert (m);
      fhg_assert (completion_handler);

      std::string name;

      {
        lock_type lock(mutex_);

        if (stopping_)
        {
          using namespace boost::system;
          completion_handler ( errc::make_error_code (errc::network_down)
                             , boost::none
                             );
          return;
        }

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
          std::pair<const message_t *, std::string> p = m_pending.front();
          m_pending.pop_front();
          *m = *p.first;
          delete p.first;
          name = p.second;
        }
      }

      using namespace boost::system;
      completion_handler (errc::make_error_code (errc::success), name);
    }

    void peer_t::connection_established (const p2p::address_t a, boost::system::error_code const &ec)
    {
      lock_type lock (mutex_);

      if (! ec)
      {
        connection_data_t & cd = connections_.find (a)->second;

        LOG( TRACE
             , p2p::to_string (my_addr_) << " (" << name_ << ")"
             << " connected to "
             << p2p::to_string (a) << " (" << cd.name << ")"
             << " @ " << cd.connection->socket().remote_endpoint();
             );

        {
          boost::asio::socket_base::keep_alive o(true);
          cd.connection->set_option (o);
        }

        // send hello message
        to_send_t to_send;
        to_send.handler = [](boost::system::error_code const&) {};
        to_send.message.header.src = my_addr_;
        to_send.message.header.dst = a;
        to_send.message.header.type_of_msg = p2p::HELLO_PACKET;
        to_send.message.header.length = name_.size();
        to_send.message.assign (name_.begin(), name_.end());

        cd.connection->start ();
        cd.o_queue.push_front (to_send);
        start_sender (a);
      }
      else
      {
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
              , "could not send message to " << p2p::to_string (a)
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

      LOG_IF(WARN, ec, "message delivery to " << p2p::to_string (a) << " failed: " << ec);

      if (! ec)
      {
        if (! cd.o_queue.empty())
        {
          // TODO: wrap in strand...
          cd.connection->async_send ( &cd.o_queue.front().message
                                    , std::bind ( &peer_t::handle_send
                                                , this
                                                , a
                                                , std::placeholders::_1
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

        fhg_assert (! cd.o_queue.empty());

        cd.send_in_progress = true;
        cd.connection->async_send ( &cd.o_queue.front().message
                                  , std::bind ( &peer_t::handle_send
                                              , this
                                              , a
                                              , std::placeholders::_1
                                              )
                                  );
      }
      catch (std::out_of_range const &)
      {
        // ignore, connection has been closed before we could start it
      }
      catch (std::exception const & ex)
      {
        LOG (ERROR, "could not start sender to " << p2p::to_string (a) << ": " << ex.what ());
      }
    }

    void peer_t::renew_kvs_entries ()
    {
      boost::asio::ip::tcp::endpoint endpoint = acceptor_.local_endpoint();

      std::string prefix ("p2p.peer");
      prefix += "." + p2p::to_string (p2p::address_t(name_));

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
        values[prefix + "." + "location" + "." + "host"]
          = boost::asio::ip::host_name();
      }

      try
      {
        _kvs_client->timed_put (values, 2 * 60 * 1000u);
      }
      catch (std::exception const &ex)
      {
        using namespace boost::system;

        m_kvs_error_handler (errc::make_error_code (errc::connection_refused));
      }

      m_renew_kvs_entries_timer.expires_from_now
        (boost::posix_time::seconds (60 + rand() % 30));

      m_renew_kvs_entries_timer.async_wait
        (std::bind (&peer_t::renew_kvs_entries, this));
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

      if (! ec && !stopping_)
      {
        fhg_assert (listen_);

        // TODO: work here schedule timeout
        backlog_.insert (listen_);

        // the connection will  call us back when it got the  hello packet or will
        // timeout
        listen_->start ();

        accept_new ();
      }
    }

    void peer_t::accept_new ()
    {
      listen_ = connection_t::ptr_t
        ( new connection_t
          ( io_service_
          , std::bind (&peer_t::handle_hello_message, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&peer_t::handle_user_data, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&peer_t::handle_error, this, std::placeholders::_1, std::placeholders::_2)
          )
        );
      listen_->local_address(my_addr_);
      acceptor_.async_accept( listen_->socket()
                            , std::bind( &peer_t::handle_accept
                                       , this
                                       , std::placeholders::_1
                                       )
                            );
    }

    void peer_t::handle_hello_message (connection_t::ptr_t c, const message_t *m)
    {
      lock_type lock (mutex_);

        if (backlog_.find (c) == backlog_.end())
        {
          LOG(ERROR, "protocol error between " << p2p::to_string (my_addr_) << " and " << p2p::to_string (m->header.src) << " closing connection");
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

          c->local_address (m->header.dst);
          std::string const remote_name (m->buf(), m->header.length);
          c->remote_address (m->header.src, remote_name);

          connection_data_t & cd = connections_[m->header.src];
          cd.name = remote_name;
          if (!cd.connection)
          {
            cd.connection = c;
          }
        }

      delete m;
    }

    void peer_t::handle_user_data
      (connection_t::ptr_t connection, const message_t *m)
    {
      fhg_assert (m);

      lock_type lock (mutex_);
      {
        if (m_to_recv.empty())
        {
          // TODO: maybe add a flag to the message indicating whether it should be delivered
          // at all costs or not
          // if (m->header.flags & IMPORTANT)
          m_pending.emplace_back (m, connection->remote_name());
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
          to_recv.handler ( errc::make_error_code (errc::success)
                          , connection->remote_name()
                          );
          lock.lock ();
        }
      }
    }

    void peer_t::handle_error (connection_t::ptr_t c, const boost::system::error_code & ec)
    {
      fhg_assert ( c != nullptr );

      lock_type lock (mutex_);

      if (connections_.find (c->remote_address()) != connections_.end())
      {
        connection_data_t & cd = connections_[c->remote_address()];

        boost::system::error_code ignore;
        c->socket ().cancel (ignore);
        c->socket ().close (ignore);

        // deactivate asynchronous sender
        cd.send_in_progress = false;

        while (! cd.o_queue.empty())
        {
          to_send_t to_send = cd.o_queue.front();
          cd.o_queue.pop_front();

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

          to_recv.message->header.src = c->remote_address();
          to_recv.message->header.dst = c->local_address();

          lock.unlock ();
          to_recv.handler (ec, c->remote_name());
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
          c->stop ();
          c.reset ();
        }
      }
    }
  }
}
