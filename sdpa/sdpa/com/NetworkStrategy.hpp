#ifndef SDPA_COM_NETWORK_STRATEGY_HPP
#define SDPA_COM_NETWORK_STRATEGY_HPP 1

#include <sdpa/logging.hpp>
#include <sdpa/events/SDPAEvent.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>
#include <boost/function.hpp>

#include <fhgcom/peer.hpp>

namespace sdpa
{
  namespace com
  {
    class NetworkStrategy
    {
    public:
      NetworkStrategy ( boost::function<void (sdpa::events::SDPAEvent::Ptr)> event_handler
                      , std::string const & peer_name
                      , fhg::com::host_t const & host
                      , fhg::com::port_t const & port
                      );
      ~NetworkStrategy();

      void perform (boost::shared_ptr<events::SDPAEvent> const & to_send);

    private:
      void handle_send (boost::shared_ptr<events::SDPAEvent> const & e, boost::system::error_code const & ec);
      void handle_recv (boost::system::error_code const & ec);

      SDPA_DECLARE_LOGGER();

      boost::function<void (sdpa::events::SDPAEvent::Ptr)> _event_handler;

      const std::string m_port;

      boost::shared_ptr<fhg::com::peer_t> m_peer;
      fhg::com::message_t m_message;
      boost::thread m_thread;

      bool m_shutting_down;
    };
  }
}

#endif
