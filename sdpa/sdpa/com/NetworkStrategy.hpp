#ifndef SDPA_COM_NETWORK_STRATEGY_HPP
#define SDPA_COM_NETWORK_STRATEGY_HPP 1

#include <sdpa/memory.hpp>

#include <boost/thread.hpp>

#include <seda/ForwardStrategy.hpp>
#include <fhgcom/peer.hpp>

namespace sdpa
{
  namespace com
  {
    class NetworkStrategy : public seda::ForwardStrategy
    {
    public:
      typedef NetworkStrategy self;
      typedef seda::ForwardStrategy super;
      typedef shared_ptr<NetworkStrategy> ptr_t;

      NetworkStrategy ( std::string const & next_stage
                      , std::string const & peer_name
                      , fhg::com::host_t const & host
                      , fhg::com::port_t const & port
                      );

      void perform (seda::IEvent::Ptr const & to_send);

      void onStageStart (std::string const & s);
      void onStageStop  (std::string const & s);
    private:
      void handle_send (seda::IEvent::Ptr const & e, boost::system::error_code const & ec);
      void handle_recv (boost::system::error_code const & ec);

      const std::string m_name;
      const std::string m_host;
      const std::string m_port;

      shared_ptr<fhg::com::peer_t> m_peer;
      fhg::com::message_t m_message;
      shared_ptr<boost::thread> m_thread;
      bool m_shutting_down;
    };
  }
}

#endif
