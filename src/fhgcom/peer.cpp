#include <fhgcom/peer.hpp>

#include <fhg/assert.hpp>
#include <util-generic/hostname.hpp>
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
    peer_t::peer_t ( std::unique_ptr<boost::asio::io_service> io_service
                   , host_t const & host
                   , port_t const & port
                   , std::function<void (p2p::address_t const&, std::string const&)> on_message
                   , std::function<void (p2p::address_t const&, std::exception_ptr const&)> on_error
                   )
      : stopping_ (false)
      , host_(host)
      , port_(port)
      , io_service_ (std::move (io_service))
      , io_service_work_(*io_service_)
      , acceptor_(*io_service_)
      , connections_()
      , _io_thread ([this] { io_service_->run(); })
    {
      try
      {
        lock_type const _ (mutex_);

        boost::asio::ip::tcp::resolver resolver(*io_service_);
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
        acceptor_.set_option (boost::asio::ip::tcp::no_delay (true));
        acceptor_.bind(endpoint);
        acceptor_.listen();

        my_addr_ = p2p::address_t ( fhg::util::hostname()
                                  , local_endpoint().port()
                                  );

        accept_new ();

        start_recv (on_message, on_error);
      }
      catch (...)
      {
        io_service_->stop();
        throw;
      }
    }

    peer_t::~peer_t()
    {
      lock_type const _ (mutex_);

      stopping_ = true;

      acceptor_.close ();

      if (listen_)
      {
        listen_->socket ().close ();
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
        delete m_pending.front();
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

      io_service_->stop();

      stopping_ = false;
    }

    p2p::address_t peer_t::connect_to (host_t const& host, port_t const& port)
    {
      boost::unique_lock<boost::recursive_mutex> const _ (mutex_);

      p2p::address_t const addr (host, port);

      if (connections_.find (addr) != connections_.end())
      {
        throw std::logic_error ( "already connected to " + std::string (host)
                               + ":" + std::string (port)
                               );
      }

      connection_data_t& cd (connections_[addr]);
      cd.connection = boost::make_shared<connection_t>
        ( *io_service_
        , std::bind (&peer_t::handle_hello_message, this, std::placeholders::_1, std::placeholders::_2)
        , std::bind (&peer_t::handle_user_data, this, std::placeholders::_1, std::placeholders::_2)
        , std::bind (&peer_t::handle_error, this, std::placeholders::_1, std::placeholders::_2)
        );
      cd.connection->local_address (my_addr_.get());
      cd.connection->remote_address (addr);

      boost::system::error_code ec;
      boost::asio::connect
        ( cd.connection->socket()
        , boost::asio::ip::tcp::resolver (*io_service_).resolve ({host, port})
        , ec
        );

      connection_established (addr, ec);

      if (ec)
      {
        throw boost::system::system_error (ec);
      }

      return addr;
    }

    void peer_t::async_send
      ( p2p::address_t const& addr
      , const std::string & data
      , std::function<void ( fhg::com::p2p::address_t const&
                           , std::exception_ptr
                           )
                     > on_error
      )
    try
    {
      lock_type lock(mutex_);

      if (stopping_)
      {
        throw boost::system::system_error
          ( boost::system::errc::make_error_code
              (boost::system::errc::network_down)
          );
      }

      // TODO: io_service_->post (...);

        connection_data_t & cd = connections_.at (addr);
        to_send_t to_send;
        to_send.message.header.src = my_addr_.get();
        to_send.message.header.dst = addr;
        to_send.message.assign (data);
        to_send.handler = [addr, on_error] (boost::system::error_code const& ec)
          {
            if (ec)
            {
              on_error (addr, std::make_exception_ptr (boost::system::system_error (ec)));
            }
          };
        cd.o_queue.push_back (to_send);

        if (cd.o_queue.size () == 1)
          start_sender (addr);
    }
    catch (...)
    {
      on_error (addr, std::current_exception());
    }

    void peer_t::start_recv
      ( std::function<void (p2p::address_t const&, std::string const&)> on_message
      , std::function<void (p2p::address_t const&, std::exception_ptr const&)> on_error
      )
    {
      async_recv ( &_incoming_message
                 , [this, on_message, on_error]
                     ( boost::system::error_code const& ec
                     , boost::optional<fhg::com::p2p::address_t> const& source
                     )
                   {
                     if (!ec)
                     {
                       on_message ( source.get()
                                  , { _incoming_message.data.begin()
                                    , _incoming_message.data.end()
                                    }
                                  );
                       start_recv (on_message, on_error);
                     }
                     else if (!!source) // == not called due to shutdown
                     {
                       on_error ( source.get()
                                , std::make_exception_ptr
                                    (boost::system::system_error (ec))
                                );
                       start_recv (on_message, on_error);
                     }
                   }
                 );
    }

    void peer_t::async_recv
      ( message_t *m
      , std::function<void ( boost::system::error_code
                           , boost::optional<fhg::com::p2p::address_t>
                           )
                     > completion_handler
      )
    {
      fhg_assert (m);
      fhg_assert (completion_handler);

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
          const message_t * p = m_pending.front();
          m_pending.pop_front();
          *m = *p;
          delete p;
        }
      }

      using namespace boost::system;
      completion_handler (errc::make_error_code (errc::success), m->header.src);
    }

    void peer_t::connection_established (const p2p::address_t a, boost::system::error_code const &ec)
    {
      lock_type lock (mutex_);

      if (! ec)
      {
        connection_data_t & cd = connections_.find (a)->second;

        {
          boost::asio::socket_base::keep_alive o(true);
          cd.connection->set_option (o);
          cd.connection->set_option (boost::asio::ip::tcp::no_delay (true));
        }

        // send hello message
        to_send_t to_send;
        to_send.handler = [](boost::system::error_code const&) {};
        to_send.message.header.src = my_addr_.get();
        to_send.message.header.dst = a;
        to_send.message.header.type_of_msg = p2p::HELLO_PACKET;
        to_send.message.resize (0);

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
        if (ec)
        {
          throw boost::system::system_error (ec);
        }
        return;
      }

      connection_data_t & cd = connections_.at (a);
      if (cd.o_queue.empty ())
      {
        if (cd.send_in_progress)
        {
          throw std::logic_error
            ("inconsistent output queue: " + ec.message());
        }
        return;
      }

      cd.o_queue.front().handler (ec);
      cd.o_queue.pop_front();

      if (ec)
      {
        throw boost::system::system_error (ec);
      }

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
          ( *io_service_
          , std::bind (&peer_t::handle_hello_message, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&peer_t::handle_user_data, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&peer_t::handle_error, this, std::placeholders::_1, std::placeholders::_2)
          )
        );
      listen_->local_address(my_addr_.get());
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
          handle_error (c, boost::system::errc::make_error_code (boost::system::errc::connection_reset));
        }
        else
        {
          backlog_.erase (c);

          c->remote_address (m->header.src);

          connection_data_t & cd = connections_[m->header.src];
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
          m_pending.emplace_back (m);
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
                          , connection->remote_address()
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
          to_recv.handler (ec, c->remote_address());
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
