#ifndef SDPA_COM_NETWORK_STRATEGY_HPP
#define SDPA_COM_NETWORK_STRATEGY_HPP 1

#include <sdpa/logging.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <fhgcom/peer.hpp>

#include <seda/Strategy.hpp>
#include <seda/Stage.hpp>

namespace sdpa
{
  namespace com
  {
    class NetworkStrategy : public seda::Strategy
    {
    public:
      typedef boost::shared_ptr<NetworkStrategy> ptr_t;

      NetworkStrategy ( seda::Stage* fallback_stage
                      , std::string const & peer_name
                      , fhg::com::host_t const & host
                      , fhg::com::port_t const & port
                      );
      ~NetworkStrategy();

      void perform (seda::IEvent::Ptr const & to_send);

    private:
      void handle_send (seda::IEvent::Ptr const & e, boost::system::error_code const & ec);
      void handle_recv (boost::system::error_code const & ec);

      SDPA_DECLARE_LOGGER();

      seda::Stage* _fallback_stage;

      const std::string m_port;

      boost::shared_ptr<fhg::com::peer_t> m_peer;
      fhg::com::message_t m_message;
      boost::thread m_thread;

      bool m_shutting_down;
    };
  }
}

#endif
