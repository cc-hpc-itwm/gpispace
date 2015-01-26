#include <sdpa/com/NetworkStrategy.hpp>

#include <csignal>

#include <sdpa/events/Codec.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <boost/lexical_cast.hpp>

#include <functional>

namespace sdpa
{
  namespace com
  {
    void NetworkStrategy::kvs_error_handler (boost::system::error_code const &)
    {
      LLOG (ERROR, _logger, "could not contact KVS, terminating");
      kill (getpid (), SIGTERM);
    }

    NetworkStrategy::NetworkStrategy ( std::function<void (sdpa::events::SDPAEvent::Ptr)> event_handler
                                     , boost::asio::io_service& peer_io_service
                                     , std::string const & peer_name
                                     , fhg::com::host_t const & host
                                     , fhg::com::port_t const & port
                                     , fhg::com::kvs::kvsc_ptr_t kvs_client
                                     )
      : _logger (fhg::log::Logger::get ("NetworkStrategy " + peer_name))
      , _event_handler (event_handler)
      , m_peer ( new fhg::com::peer_t
                 ( peer_io_service
                 , peer_name
                 , fhg::com::host_t (host)
                 , fhg::com::port_t (port)
                 , kvs_client
                 , std::bind ( &NetworkStrategy::kvs_error_handler
                             , this
                             , std::placeholders::_1
                             )
                 )
               )
      , m_message()
      , m_thread (&fhg::com::peer_t::run, m_peer)
      , m_shutting_down (false)
    {
      m_peer->start ();
      m_peer->async_recv
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

      m_peer->stop();
      m_thread.join();
    }

    void NetworkStrategy::perform (boost::shared_ptr<events::SDPAEvent> const & sdpa_event)
    {
      static sdpa::events::Codec codec;

      // convert event to fhg::com::message_t

      fhg::com::message_t msg;
      msg.header.dst = fhg::com::p2p::address_t (sdpa_event->to());
      msg.header.src = m_peer->address();

      const std::string encoded_evt (codec.encode(sdpa_event.get()));
      msg.data.assign (encoded_evt.begin(), encoded_evt.end());
      msg.header.length = msg.data.size();

      try
      {
        m_peer->async_send
          ( &msg
          , std::bind
            (&NetworkStrategy::handle_send, this, sdpa_event, std::placeholders::_1)
          );
      }
      catch (std::exception const & ex)
      {
        sdpa::events::ErrorEvent::Ptr ptrErrEvt
          (new sdpa::events::ErrorEvent( sdpa_event->to()
                                       , sdpa_event->from()
                                       , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                       , ex.what())
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
        sdpa::events::ErrorEvent::Ptr ptrErrEvt
          (new sdpa::events::ErrorEvent( sdpa_event->to()
                                       , sdpa_event->from()
                                       , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                       , ec.message())
          );
        _event_handler (ptrErrEvt);
      }
    }

    void NetworkStrategy::handle_recv ( boost::system::error_code const & ec
                                      , boost::optional<std::string> source_name
                                      )
    {
      static sdpa::events::Codec codec;

      if (! ec)
      {
        // convert m_message to event
        sdpa::events::SDPAEvent::Ptr evt
          (codec.decode (std::string (m_message.data.begin(), m_message.data.end())));
        _event_handler (evt);

        m_peer->async_recv
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
        if (m_message.header.src != m_peer->address())
        {
          sdpa::events::ErrorEvent::Ptr
            error(new sdpa::events::ErrorEvent ( source_name.get_value_or ("unknown source")
                                                , m_peer->name()
                                                , (ec == boost::asio::error::eof) // Connection closed cleanly by peer
                                                ? sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                                                : sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                                , ec.message()
                                                )
                );
          _event_handler (error);

          m_peer->async_recv
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
