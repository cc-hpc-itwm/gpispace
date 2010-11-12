#include "NetworkStrategy.hpp"

#include <sdpa/events/Codec.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <boost/lexical_cast.hpp>

namespace sdpa
{
  namespace com
  {
    NetworkStrategy::NetworkStrategy ( std::string const & next_stage
                                     , std::string const & peer_name
                                     , fhg::com::host_t const & host
                                     , fhg::com::port_t const & port
                                     )
      : super (next_stage)
      , m_peer (peer_name, host, port)
      , m_shutting_down (false)
    {}

    NetworkStrategy::~NetworkStrategy ()
    {}

    void NetworkStrategy::perform (seda::IEvent::Ptr const & e)
    {
      static sdpa::events::Codec codec;

      assert (e);

      // convert event to fhg::com::message_t
      if (sdpa::events::SDPAEvent *sdpa_event = dynamic_cast<sdpa::events::SDPAEvent*>(e.get()))
      {
        DLOG(TRACE, "sending event: " << sdpa_event->str());

        fhg::com::message_t msg;
        msg.header.dst = m_peer.resolve_name (sdpa_event->to());
        msg.header.src = m_peer.address();

        const std::string encoded_evt (codec.encode(sdpa_event));
        msg.data.assign (encoded_evt.begin(), encoded_evt.end());
        msg.header.length = msg.data.size();

        m_peer.async_send (&msg, boost::bind (&self::handle_send, this, _1));
      }
      else
      {
        LOG(ERROR, "cannot handle non-SDPAEvent events: " << e->str());
      }
    }

    void NetworkStrategy::onStageStart (std::string const &s)
    {
      m_shutting_down = false;

      super::onStageStart (s);

      m_thread.reset (new boost::thread(boost::bind(&fhg::com::peer_t::run, &m_peer)));
      m_peer.start ();
      m_peer.async_recv (&m_message, boost::bind(&self::handle_recv, this, _1));
    }

    void NetworkStrategy::onStageStop (std::string const & s)
    {
      m_shutting_down = true;

      m_peer.stop();
      m_thread->join ();

      super::onStageStop (s);
    }

    void NetworkStrategy::handle_send (boost::system::error_code const & ec)
    {
      LOG_IF(WARN, ec, "sending failed");
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
          super::perform (evt);
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "could not handle incoming message: " << ex.what());
        }

        m_peer.async_recv (&m_message, boost::bind(&self::handle_recv, this, _1));
      }
      else if (! m_shutting_down)
      {
        const fhg::com::p2p::address_t & addr = m_message.header.src;
        if (addr != m_peer.address())
        {
          LOG(WARN, "connection to " << m_peer.resolve (addr, "*unknown*") << " lost: " << ec);

          sdpa::events::ErrorEvent::Ptr
            error(new sdpa::events::ErrorEvent ( m_peer.resolve(addr, "*unknown*")
                                               , m_peer.name()
                                               , sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                                               , boost::lexical_cast<std::string>(ec)
                                               )
                 );
          super::perform (error);
          m_peer.async_recv (&m_message, boost::bind(&self::handle_recv, this, _1));
        }
        else
        {
          LOG(TRACE, m_peer.name() << " is shutting down");
        }
      }
    }
  }
}
