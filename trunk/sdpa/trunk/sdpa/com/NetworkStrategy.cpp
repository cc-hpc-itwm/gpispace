#include "NetworkStrategy.hpp"

namespace sdpa
{
  namespace com
  {
    NetworkStrategy::NetworkStrategy ( std::string const & next_stage
                                     , std::string const & peer_name
                                     , fhg::com::host_t const & host
                                     , fhg::com::port_t const & port
                                     )
      : seda::ForwardStrategy (next_stage)
      , m_peer (peer_name, host, port)
    {}

    NetworkStrategy::~NetworkStrategy ()
    {}

    void NetworkStrategy::perform (seda::IEvent::Ptr const & e)
    {
      // convert event to fhg::com::message_t
      // synchronous send
    }

    void NetworkStrategy::onStageStart (std::string const &)
    {
      m_thread.reset (new boost::thread(boost::bind(&fhg::com::peer_t::run, &m_peer)));
      m_peer.start ();
      m_peer.async_recv (&m_message, boost::bind(&self::handle_recv, this, _1, _2));
    }

    void NetworkStrategy::onStageStop (std::string const &)
    {
      m_peer.stop();
      m_thread->join ();
    }

    void NetworkStrategy::handle_recv (fhg::com::p2p::address_t addr, boost::system::error_code const & ec)
    {
      if (! ec)
      {
        // convert m_message to event
        // ForwardStrategy::perform (event)

        m_peer.async_recv (&m_message, boost::bind(&self::handle_recv, this, _1, _2));
      }
      else // if (ec != eof)
      {
        // TODO: add handlers to peer to figure out which connection had problems
        // ForwardStrategy::perform (error)
      }
    }
  }
}

