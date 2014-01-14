#include "NetworkStrategy.hpp"
#include <fhg/assert.hpp>

#include <csignal>

#include <sdpa/events/Codec.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <boost/lexical_cast.hpp>

namespace sdpa
{
  namespace com
  {
    void NetworkStrategy::kvs_error_handler (boost::system::error_code const &)
    {
      LLOG (ERROR, _logger, "could not contact KVS, terminating");
      kill (getpid (), SIGTERM);
    }

    NetworkStrategy::NetworkStrategy ( boost::function<void (sdpa::events::SDPAEvent::Ptr)> event_handler
                                     , std::string const & peer_name
                                     , fhg::com::host_t const & host
                                     , fhg::com::port_t const & port
                                     )
      : _logger (fhg::log::Logger::get ("NetworkStrategy " + peer_name))
      , _event_handler (event_handler)
      , m_peer ( new fhg::com::peer_t ( peer_name
                                      , fhg::com::host_t (host)
                                      , fhg::com::port_t (port)
                                      )
               )
      , m_message()
      , m_thread (&fhg::com::peer_t::run, m_peer)
      , m_shutting_down (false)
    {
      m_peer->set_kvs_error_handler
        (boost::bind (&NetworkStrategy::kvs_error_handler, this, _1));
      m_peer->start ();
      m_peer->async_recv (&m_message, boost::bind(&NetworkStrategy::handle_recv, this, _1));
    }

    NetworkStrategy::~NetworkStrategy()
    {
      m_shutting_down = true;

      m_peer->stop();
      m_thread.join();
    }

    void NetworkStrategy::perform (boost::shared_ptr<events::SDPAEvent> const & sdpa_event)
    {
      static sdpa::events::Codec codec;

      // convert event to fhg::com::message_t

      DLLOG(TRACE, _logger, "sending event: " << sdpa_event->str());

      fhg::com::message_t msg;
      msg.header.dst = m_peer->resolve_name (sdpa_event->to());
      msg.header.src = m_peer->address();

      const std::string encoded_evt (codec.encode(sdpa_event.get()));
      msg.data.assign (encoded_evt.begin(), encoded_evt.end());
      msg.header.length = msg.data.size();

      try
      {
        m_peer->async_send (&msg, boost::bind (&NetworkStrategy::handle_send, this, sdpa_event, _1));
      }
      catch (std::exception const & ex)
      {
        sdpa::events::ErrorEvent::Ptr ptrErrEvt
          (new sdpa::events::ErrorEvent( sdpa_event->to()
                                       , sdpa_event->from()
                                       , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                       , sdpa_event->str())
          );
        _event_handler (ptrErrEvt);
      }
    }

    void NetworkStrategy::handle_send ( boost::shared_ptr<events::SDPAEvent> const &sdpa_event
                                      , boost::system::error_code const & ec
                                      )
    {
      if (ec)
      {
        DLLOG ( WARN
              , _logger
              , "send failed:"
              << " ec := " << ec
              << " msg := " << ec.message ()
              << " event := " << sdpa_event->str()
              << " to := " << sdpa_event->to ()
              << " from := " << sdpa_event->from ()
              );

        //sdpa::events::SDPAEvent::Ptr err (sdpa_event->create_reply (ec));
        sdpa::events::ErrorEvent::Ptr ptrErrEvt
          (new sdpa::events::ErrorEvent( sdpa_event->to()
                                       , sdpa_event->from()
                                       , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                       , sdpa_event->str())
          );
        _event_handler (ptrErrEvt);
      }
    }

    void NetworkStrategy::handle_recv (boost::system::error_code const & ec)
    {
      static sdpa::events::Codec codec;

      if (! ec)
      {
        // convert m_message to event
        try
        {
          sdpa::events::SDPAEvent::Ptr evt
            (codec.decode (std::string (m_message.data.begin(), m_message.data.end())));
          DLLOG(TRACE, _logger, "received event: " << evt->str());
          _event_handler (evt);
        }
        catch (std::exception const & ex)
        {
          LLOG(WARN, _logger, "could not handle incoming message: " << ex.what());
        }

        m_peer->async_recv (&m_message, boost::bind(&NetworkStrategy::handle_recv, this, _1));
      }
      else if (! m_shutting_down)
      {
        const fhg::com::p2p::address_t & addr = m_message.header.src;
        if (addr != m_peer->address())
        {
          sdpa::events::ErrorEvent::Ptr
            error(new sdpa::events::ErrorEvent ( m_peer->resolve(addr, "*unknown*")
                                               , m_peer->name()
                                               , sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                                               , boost::lexical_cast<std::string>(ec)
                                               )
                 );
          _event_handler (error);
          m_peer->async_recv (&m_message, boost::bind(&NetworkStrategy::handle_recv, this, _1));
        }
      }
    }
  }
}
