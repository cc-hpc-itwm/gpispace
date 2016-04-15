#include <sdpa/com/NetworkStrategy.hpp>

#include <csignal>

#include <sdpa/events/ErrorEvent.hpp>

#include <boost/lexical_cast.hpp>

#include <functional>

namespace sdpa
{
  namespace com
  {
    NetworkStrategy::NetworkStrategy ( std::function<void (fhg::com::p2p::address_t const&, sdpa::events::SDPAEvent::Ptr)> event_handler
                                     , std::function<void (fhg::com::p2p::address_t const&, std::exception_ptr const&)> on_error
                                     , std::unique_ptr<boost::asio::io_service> peer_io_service
                                     , fhg::com::host_t const & host
                                     , fhg::com::port_t const & port
                                     )
      : _codec()
      , _event_handler (event_handler)
      , _on_error (std::move (on_error))
      , _peer (std::move (peer_io_service), host, port)
    {
      _peer.start_recv
        ( [this] ( fhg::com::p2p::address_t const& source
                 , std::string const& data
                 )
          {
            _event_handler
              (source, sdpa::events::SDPAEvent::Ptr (_codec.decode (data)));
          }
        , _on_error
        );
    }

    fhg::com::p2p::address_t NetworkStrategy::connect_to
      (fhg::com::host_t const& host, fhg::com::port_t const& port)
    {
      return _peer.connect_to (host, port);
    }
  }
}
