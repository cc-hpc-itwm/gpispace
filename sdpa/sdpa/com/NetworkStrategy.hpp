#ifndef SDPA_COM_NETWORK_STRATEGY_HPP
#define SDPA_COM_NETWORK_STRATEGY_HPP 1

#include <sdpa/events/SDPAEvent.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <fhgcom/peer.hpp>

#include <functional>

namespace sdpa
{
  namespace com
  {
    class NetworkStrategy
    {
    public:
      NetworkStrategy ( std::function<void (sdpa::events::SDPAEvent::Ptr)> event_handler
                      , boost::asio::io_service& peer_io_service
                      , std::string const & peer_name
                      , fhg::com::host_t const & host
                      , fhg::com::port_t const & port
                      , fhg::com::kvs::kvsc_ptr_t kvs_client
                      );
      ~NetworkStrategy();

      void perform (boost::shared_ptr<events::SDPAEvent> const & to_send);

      boost::asio::ip::tcp::endpoint local_endpoint() const
      {
        return m_peer->local_endpoint();
      }

    private:
      void handle_send (boost::shared_ptr<events::SDPAEvent> const & e, boost::system::error_code const & ec);
      void handle_recv ( boost::system::error_code const & ec
                       , boost::optional<std::string> source_name
                       );

      void kvs_error_handler (boost::system::error_code const &);

      fhg::log::Logger::ptr_t _logger;

      std::function<void (sdpa::events::SDPAEvent::Ptr)> _event_handler;

      const std::string m_port;

      boost::shared_ptr<fhg::com::peer_t> m_peer;
      fhg::com::message_t m_message;
      boost::thread m_thread;

      bool m_shutting_down;
    };
  }
}

#endif
