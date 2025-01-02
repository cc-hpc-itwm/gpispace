// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <fhgcom/peer.hpp>

#include <fhg/assert.hpp>
#include <util-generic/hostname.hpp>

#include <boost/asio/connect.hpp>
// should only need ssl/context.hpp, but that's missing an include
#include <boost/asio/ssl.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/make_shared.hpp>
#include <boost/system/system_error.hpp>

#include <fmt/core.h>
#include <future>
#include <stdexcept>
#include <utility>

namespace
{
  //Note: this mutex is needed to ensure non-concurrent
  //initialization of the ::boost::ssl:context in peers
  std::mutex ssl_context_threadunsafety_guard;
}

namespace fhg
{
  namespace com
  {
#define REQUIRE_ON_STRAND()                                             \
    do                                                                  \
    {                                                                   \
      if (!strand_.running_in_this_thread())                            \
      {                                                                 \
        throw std::logic_error                                          \
          ( std::string ("Function '")                                  \
          + __PRETTY_FUNCTION__                                         \
          + "' requires being called from within strand, but isn't."    \
          );                                                            \
      }                                                                 \
    }                                                                   \
    while (false)

    peer_t::peer_t ( std::unique_ptr<::boost::asio::io_service> io_service
                   , host_t const& host
                   , port_t const& port
                   , gspc::Certificates const& certificates
                   )
      : io_service_ (std::move (io_service))
      , strand_ (*io_service_)
      , io_service_work_(*io_service_)
      , acceptor_(*io_service_)
      , connections_()
      , TESTING_ONLY_handshake_exception_ (nullptr)
      , _io_thread ([this] { io_service_->run(); })
    {
      try
      {
        lock_type const _ (mutex_);

        if (certificates.path.has_value())
        {
          std::lock_guard<std::mutex> lock_context
            (ssl_context_threadunsafety_guard);
          ctx_ = std::make_unique<::boost::asio::ssl::context>
                   (*io_service_, ::boost::asio::ssl::context::sslv23);

          ctx_->set_options ( ::boost::asio::ssl::context::default_workarounds
                            | ::boost::asio::ssl::context::no_sslv2
                            | ::boost::asio::ssl::context::no_sslv3
                            | ::boost::asio::ssl::context::single_dh_use
                            );

          ctx_->use_certificate_chain_file
            (std::filesystem::canonical (certificates.path.value()/"server.crt").string());

          ctx_->use_private_key_file
           ( std::filesystem::canonical (certificates.path.value()/"server.key").string()
           , ::boost::asio::ssl::context::pem
           );
          ctx_->use_tmp_dh_file
            (std::filesystem::canonical (certificates.path.value()/"dh2048.pem").string());

          ctx_->set_verify_mode
            ( ::boost::asio::ssl::context::verify_fail_if_no_peer_cert
            | ::boost::asio::ssl::context::verify_peer
            );

          ctx_->load_verify_file
            (std::filesystem::canonical (certificates.path.value()/"server.crt").string());
        }

        ::boost::asio::ip::tcp::resolver resolver(*io_service_);
        ::boost::asio::ip::tcp::resolver::query query
            (host, to_string (port));
        ::boost::asio::ip::tcp::endpoint endpoint = *resolver.resolve (query);

        if (static_cast<std::string> (host) == "*")
        {
            endpoint.address (::boost::asio::ip::address_v4::any());
        }

        acceptor_.open (endpoint.protocol());
        acceptor_.set_option (::boost::asio::ip::tcp::acceptor::reuse_address (true));
        acceptor_.set_option (::boost::asio::ip::tcp::no_delay (true));
        acceptor_.bind (endpoint);
        acceptor_.listen();

        my_addr_ = p2p::address_t
          (host_t {fhg::util::hostname()}, port_t {local_endpoint().port()});

        strand_.post ([this] { accept_new(); });
      }
      catch (...)
      {
        io_service_->stop();
        throw;
      }
    }

    peer_t::~peer_t()
    {
      stopping_ = true;

      auto cancels_done (std::make_shared<std::promise<void>>());

      strand_.dispatch
        ( [this, cancels_done]
          {
            lock_type const lock (mutex_);

            acceptor_.close ();

            using namespace ::boost::system;
            auto const error_code
              (errc::make_error_code (errc::operation_canceled));

            if (listen_)
            {
              handle_error (listen_, error_code);
            }

            while (!connections_.empty())
            {
              handle_error
                (connections_.begin()->second.connection, error_code);
            }

            while (!backlog_.empty())
            {
              handle_error (*backlog_.begin(), error_code);
            }

            cancels_done->set_value();
          }
        );

      cancels_done->get_future().wait();

      io_service_->stop();
    }

    p2p::address_t peer_t::connect_to (host_t const& host, port_t const& port)
    {
      p2p::address_t const addr (host, port);

      {
        std::unique_lock<std::recursive_mutex> const _ (mutex_);

        if (connections_.find (addr) != connections_.end())
        {
          throw std::logic_error
            { fmt::format
                ( "already connected to {}:{}"
                , static_cast<std::string> (host)
                , to_string (port)
                )
            };
        }
      }

      // \note Only detects `hostname` (as we use that to create
      // my_addr_), not `localhost` or `::1` or equivalent
      // ones. Those will just hang as we can't connect and accept
      // at the same time with SSL (handshake).
      if (addr == my_addr_.get())
      {
        throw std::logic_error ("unable to connect to self");
      }

      auto connect_done
        (std::make_shared<std::promise<std::exception_ptr>>());

      connections_.emplace (addr, connection_data_t());

      strand_.dispatch
        ( [this, addr, host, port, connect_done]
          {
            try
            {
              std::unique_lock<std::recursive_mutex> const _ (mutex_);

              connection_data_t& cd (connections_.at (addr));

              cd.connection = ::boost::make_shared<connection_t>
                ( *io_service_
                , ctx_.get()
                , strand_
                , std::bind (&peer_t::handle_hello_message, this, std::placeholders::_1, std::placeholders::_2)
                , std::bind (&peer_t::handle_user_data, this, std::placeholders::_1, std::placeholders::_2)
                , std::bind (&peer_t::handle_error, this, std::placeholders::_1, std::placeholders::_2)
                , this
                );
              cd.connection->local_address (my_addr_.get());
              cd.connection->remote_address (addr);

              ::boost::asio::connect
                ( cd.connection->socket()
                , ::boost::asio::ip::tcp::resolver (*io_service_)
                    .resolve ({host, to_string (port)})
                );

              cd.connection->request_handshake (std::move (connect_done));
              // control flow continues in `request_handshake_response`.
            }
            catch (...)
            {
              connect_done->set_value (std::current_exception());
            }
          }
        );

      auto const exception_during_connect
        (connect_done->get_future().get());
      if (exception_during_connect)
      {
        std::rethrow_exception (exception_during_connect);
      }

      return addr;
    }

    void peer_t::request_handshake_response
      ( p2p::address_t addr
      , std::shared_ptr<std::promise<std::exception_ptr>> connect_done
      , ::boost::system::error_code const& ec
      )
    {
      REQUIRE_ON_STRAND();

      try
      {
        lock_type const lock (mutex_);

        if (ec)
        {
          throw handshake_exception (ec);
        }

        connection_established (lock, connections_.at (addr));

        connect_done->set_value (nullptr);
      }
      catch (...)
      {
        connect_done->set_value (std::current_exception());
      }
    }

    void peer_t::send ( p2p::address_t const& addr
                      , std::string const& data
                      )
    {
      using async_op_t = std::promise<::boost::system::error_code>;
      async_op_t send_finished;
      async_send
        ( addr, data
        , [&] (auto ec) { send_finished.set_value (ec); }
        );

      const ::boost::system::error_code ec (send_finished.get_future().get());
      if (ec)
      {
        throw ::boost::system::system_error (ec);
      }
    }

    void peer_t::async_send ( p2p::address_t const& addr
                            , std::string const& data
                            , peer_t::handler_t completion_handler
                            )
    {
      fhg_assert (completion_handler);

      lock_type const lock (mutex_);

      if (stopping_)
      {
        using namespace ::boost::system;
        completion_handler (errc::make_error_code (errc::network_down));
        return;
      }

      // TODO: io_service_->post (...);

      connection_data_t & cd = connections_.at (addr);
      cd.o_queue.push_back ({{data, my_addr_.get(), addr}, completion_handler});

      if (cd.o_queue.size () == 1)
      {
        start_sender (lock, cd);
      }
    }

    peer_t::Received::Received (::boost::system::error_code ec)
      : _ec (std::move (ec))
    {}
    peer_t::Received::Received
       ( ::boost::system::error_code ec
       , p2p::address_t source
       , message_t message
       )
         : _ec (std::move (ec))
         , _source (std::move (source))
         , _message (std::move (message))
    {}
    ::boost::system::error_code peer_t::Received::ec() const
    {
      return _ec;
    }
    p2p::address_t const& peer_t::Received::source() const
    {
      if (!_source)
      {
        throw std::logic_error ("peer_t::Received::source");
      }

      return *_source;
    }
    message_t const& peer_t::Received::message() const
    {
      if (!_message)
      {
        throw std::logic_error ("peer_t::Received::message");
      }

      return *_message;
    }

    void peer_t::async_recv (std::function<void (Received)> completion_handler)
    {
      fhg_assert (completion_handler);

      lock_type lock (mutex_);

      if (stopping_)
      {
        using namespace ::boost::system;
        completion_handler (errc::make_error_code (errc::network_down));
      }
      else if (m_pending.empty())
      {
        // TODO: implement async receive on connection!
        // \todo Figure out what that todo means.
        m_to_recv.emplace_back (std::move (completion_handler));
      }
      else
      {
        message_t m (std::move (m_pending.front()));
        m_pending.pop_front();

        // \note Allow for recursive calling of async_recv.
        // \todo Instead, always post onto strand and never call
        // handler directly?
        lock.unlock();

        auto const src {m.header.src};

        using namespace ::boost::system;
        completion_handler
          ( { errc::make_error_code (errc::success)
            , src
            , std::move (m)
            }
          );
      }
    }

    void peer_t::connection_established
      (lock_type const& lock, connection_data_t& cd)
    {
      cd.connection->set_option (::boost::asio::socket_base::keep_alive (true));
      cd.connection->set_option (::boost::asio::ip::tcp::no_delay (true));

      // send hello message
      cd.connection->start ();
      cd.o_queue.push_front
        ( { {message_t::Hello{}, my_addr_.get(), cd.connection->remote_address()}
          , [](::boost::system::error_code const&) {}
          }
        );
      start_sender (lock, cd);
    }

    void peer_t::handle_send (p2p::address_t a, ::boost::system::error_code const& ec)
    {
      lock_type lock (mutex_);

      if (connections_.find (a) == connections_.end())
      {
        if (ec)
        {
          throw ::boost::system::system_error (ec);
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
        throw ::boost::system::system_error (ec);
      }

      if (! ec)
      {
        if (! cd.o_queue.empty())
        {
          cd.connection->async_send ( std::move (cd.o_queue.front().message)
                                    , strand_.wrap
                                        ( std::bind ( &peer_t::handle_send
                                                    , this
                                                    , a
                                                    , std::placeholders::_1
                                                    )
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

    void peer_t::start_sender (lock_type const&, connection_data_t& cd)
    {
      if (cd.send_in_progress || cd.o_queue.empty())
      {
        return;
      }

      fhg_assert (! cd.o_queue.empty());

      cd.send_in_progress = true;
      cd.connection->async_send
        ( std::move (cd.o_queue.front().message)
        , strand_.wrap
            ( std::bind ( &peer_t::handle_send
                        , this
                        , cd.connection->remote_address()
                        , std::placeholders::_1
                        )
            )
        );
    }

    void peer_t::handle_accept (::boost::system::error_code const& ec)
    {
      if (! ec && !stopping_)
      {
        fhg_assert (listen_);

        listen_->acknowledge_handshake();
        // control flow continues in `acknowledge_handshake_response`.
      }
    }

    void peer_t::acknowledge_handshake_response
      (connection_t::ptr_t connection, ::boost::system::error_code const& ec)
    {
      REQUIRE_ON_STRAND();

      // \todo Allow accepting a new connection while still
      // handshaking this one: denial of service attack possible.
      fhg_assert (connection == listen_);

      if (ec)
      {
        TESTING_ONLY_handshake_exception_
          = std::make_exception_ptr (handshake_exception (ec));
      }
      else
      {
        // TODO: work here schedule timeout
        backlog_.insert (connection);

        // the connection will call us back when it got the hello packet
        // or will timeout
        connection->start();
      }

      accept_new();
    }

    void peer_t::accept_new ()
    {
      REQUIRE_ON_STRAND();

      listen_ = connection_t::ptr_t
        ( new connection_t
          ( *io_service_
          , ctx_.get()
          , strand_
          , std::bind (&peer_t::handle_hello_message, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&peer_t::handle_user_data, this, std::placeholders::_1, std::placeholders::_2)
          , std::bind (&peer_t::handle_error, this, std::placeholders::_1, std::placeholders::_2)
          , this
          )
        );
      listen_->local_address (my_addr_.get());
      acceptor_.async_accept ( listen_->socket()
                             , strand_.wrap
                                 ( std::bind ( &peer_t::handle_accept
                                             , this
                                             , std::placeholders::_1
                                             )
                                 )
                             );
    }

    void peer_t::handle_hello_message (connection_t::ptr_t c, std::unique_ptr<message_t> m)
    {
      REQUIRE_ON_STRAND();

      lock_type lock (mutex_);

      if (backlog_.find (c) == backlog_.end())
      {
        handle_error (c, ::boost::system::errc::make_error_code (::boost::system::errc::connection_reset));
      }
      else
      {
        backlog_.erase (c);

        c->local_address (m->header.dst);
        c->remote_address (m->header.src);

        connection_data_t & cd = connections_[m->header.src];
        if (!cd.connection)
        {
          cd.connection = c;
        }
      }
    }

    void peer_t::handle_user_data
      (connection_t::ptr_t connection, std::unique_ptr<message_t> m)
    {
      REQUIRE_ON_STRAND();

      fhg_assert (m);

      lock_type lock (mutex_);
      {
        if (m_to_recv.empty())
        {
          // TODO: maybe add a flag to the message indicating whether it should be delivered
          // at all costs or not
          // if (m->header.flags & IMPORTANT)
          m_pending.emplace_back (std::move (*m));
        }
        else
        {
          auto const to_recv (std::move (m_to_recv.front()));
          m_to_recv.pop_front();

          using namespace ::boost::system;

          lock.unlock ();
          to_recv ( { errc::make_error_code (errc::success)
                    , connection->remote_address()
                    , std::move (*m)
                    }
                  );
          lock.lock ();
        }
      }
    }

    void peer_t::handle_error (connection_t::ptr_t c, ::boost::system::error_code const& ec)
    {
      REQUIRE_ON_STRAND();

      fhg_assert ( c != nullptr );

      lock_type lock (mutex_);

      if (connections_.find (c->remote_address()) != connections_.end())
      {
        connection_data_t & cd = connections_.at (c->remote_address());

        c->stop();

        // deactivate asynchronous sender
        cd.send_in_progress = false;

        while (! cd.o_queue.empty())
        {
          to_send_t to_send (std::move (cd.o_queue.front()));
          cd.o_queue.pop_front();

          lock.unlock ();
          to_send.handler (ec);
          lock.lock ();
        }

        // \todo Instead, deliver them? The sender may assume they
        // were delivered as there was no error in sending.
        m_pending.remove_if
          ( [&] (message_t const& message)
            {
              return message.header.dst == c->remote_address();
            }
          );

        // the handler might async recv again...
        std::list<std::function<void (Received)>> tmp;
        std::swap (tmp, m_to_recv);

        while (! tmp.empty())
        {
          auto const to_recv (std::move (tmp.front()));
          tmp.pop_front();

          message_t m {c->remote_address(), c->local_address()};

          lock.unlock ();
          to_recv ({ec, c->remote_address(), std::move (m)});
          lock.lock ();
        }

        connections_.erase (c->remote_address());
      }
      else if (backlog_.find (c) != backlog_.end ())
      {
        backlog_.erase (c);
      }
      else // should be listen_
      {
        c->stop ();
      }
    }

    std::exception_ptr peer_t::TESTING_ONLY_handshake_exception() const
    {
      return TESTING_ONLY_handshake_exception_;
    }
  }
}
