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
    static void kvs_error_handler (boost::system::error_code const &)
    {
      MLOG (ERROR, "could not contact KVS, terminating");
      kill (getpid (), SIGTERM);
    }

    NetworkStrategy::NetworkStrategy ( seda::Stage::Ptr fallback_stage
                                     , std::string const & peer_name
                                     , fhg::com::host_t const & host
                                     , fhg::com::port_t const & port
                                     )
      : SDPA_INIT_LOGGER ("NetworkStrategy " + peer_name)
      , _fallback_stage (fallback_stage)
      , m_peer ( new fhg::com::peer_t ( peer_name
                                      , fhg::com::host_t (host)
                                      , fhg::com::port_t (port)
                                      )
               )
      , m_message()
      , m_thread (&fhg::com::peer_t::run, m_peer)
      , m_shutting_down (false)
    {
      m_peer->set_kvs_error_handler (kvs_error_handler);
      m_peer->start ();
      m_peer->async_recv (&m_message, boost::bind(&NetworkStrategy::handle_recv, this, _1));
    }

    void NetworkStrategy::perform (seda::IEvent::Ptr const & e)
    {
      static sdpa::events::Codec codec;

      assert (e);

      sdpa::events::SDPAEvent* sdpa_event
        (dynamic_cast<sdpa::events::SDPAEvent*>(e.get()));

      assert (sdpa_event);

      // convert event to fhg::com::message_t

      DLOG(TRACE, "sending event: " << sdpa_event->str());

      fhg::com::message_t msg;
      msg.header.dst = m_peer->resolve_name (sdpa_event->to());
      msg.header.src = m_peer->address();

      const std::string encoded_evt (codec.encode(sdpa_event));
      msg.data.assign (encoded_evt.begin(), encoded_evt.end());
      msg.header.length = msg.data.size();

      try
      {
        m_peer->async_send (&msg, boost::bind (&NetworkStrategy::handle_send, this, e, _1));
      }
      catch (std::exception const & ex)
      {
        sdpa::events::ErrorEvent::Ptr ptrErrEvt
          (new sdpa::events::ErrorEvent( sdpa_event->to()
                                       , sdpa_event->from()
                                       , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                       , sdpa_event->str())
          );
        _fallback_stage->send (ptrErrEvt);
      }
    }

    void NetworkStrategy::onStageStop()
    {
      m_shutting_down = true;

      m_peer->stop();
      m_thread.join();
      m_peer.reset();

      _fallback_stage.reset();
    }

    void NetworkStrategy::handle_send ( seda::IEvent::Ptr const &e
                                      , boost::system::error_code const & ec
                                      )
    {
      if (ec)
      {
        sdpa::events::SDPAEvent* sdpa_event
          (dynamic_cast<sdpa::events::SDPAEvent*>(e.get()));

        assert (sdpa_event);

        DMLOG ( WARN
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
        _fallback_stage->send (ptrErrEvt);
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
          DLOG(TRACE, "received event: " << evt->str());
          _fallback_stage->send (evt);
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "could not handle incoming message: " << ex.what());
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
          _fallback_stage->send (error);
          m_peer->async_recv (&m_message, boost::bind(&NetworkStrategy::handle_recv, this, _1));
        }
      }
    }
  }
}
