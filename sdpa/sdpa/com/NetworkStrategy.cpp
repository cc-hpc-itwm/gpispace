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
                                     , std::unique_ptr<boost::asio::io_service> peer_io_service
                                     , fhg::com::host_t const & host
                                     , fhg::com::port_t const & port
                                     )
      : _codec()
      , _event_handler (event_handler)
      , m_message()
      , m_shutting_down (false)
      , _peer (std::move (peer_io_service), host, port)
    {
      _peer.async_recv
        ( &m_message
        , std::bind ( &NetworkStrategy::handle_recv
                    , this
                    , std::placeholders::_1
                    , std::placeholders::_2
                    )
        );
    }

    NetworkStrategy::~NetworkStrategy()
    {
      m_shutting_down = true;
    }

    fhg::com::p2p::address_t NetworkStrategy::connect_to
      (fhg::com::host_t const& host, fhg::com::port_t const& port)
    {
      return _peer.connect_to (host, port);
    }

    void NetworkStrategy::handle_recv ( boost::system::error_code const & ec
                                      , boost::optional<fhg::com::p2p::address_t> source
                                      )
    {
      if (! ec)
      {
        // convert m_message to event
        sdpa::events::SDPAEvent::Ptr evt
          (_codec.decode (std::string (m_message.data.begin(), m_message.data.end())));
        _event_handler (source.get(), evt);

        _peer.async_recv
          ( &m_message
          , std::bind ( &NetworkStrategy::handle_recv
                      , this
                      , std::placeholders::_1
                      , std::placeholders::_2
                      )
          );
      }
      else if (! m_shutting_down)
      {
        if (m_message.header.src != _peer.address())
        {
          sdpa::events::ErrorEvent::Ptr
            error(new sdpa::events::ErrorEvent ( (ec == boost::asio::error::eof) // Connection closed cleanly by peer
                                                ? sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                                                : sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                                , ec.message()
                                                )
                );
          _event_handler
            (source.get_value_or (fhg::com::p2p::address_t ("unknown")), error);

          _peer.async_recv
           ( &m_message
           , std::bind ( &NetworkStrategy::handle_recv
                       , this
                       , std::placeholders::_1
                       , std::placeholders::_2
                       )
           );
        }
      }
    }
  }
}
