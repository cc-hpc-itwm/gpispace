#include <sdpa/client.hpp>

#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/events/put_token.hpp>

#include <fhg/util/macros.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <functional>

namespace sdpa
{
  namespace client
  {
    Client::Client ( fhg::com::host_t const& orchestrator_host
                   , fhg::com::port_t const& orchestrator_port
                   , std::unique_ptr<boost::asio::io_service> peer_io_service
                   )
      : _stopping (false)
      , m_peer ( std::move (peer_io_service)
               , fhg::com::host_t ("*")
               , fhg::com::port_t ("0")
               )
      , _drts_entrypoint_address
          (m_peer.connect_to (orchestrator_host, orchestrator_port))
    {
      m_peer.async_recv ( &m_message
                        , std::bind ( &Client::handle_recv
                                    , this
                                    , std::placeholders::_1
                                    , std::placeholders::_2
                                    )
                        );
    }

    Client::~Client()
    {
      _stopping = true;
    }

    void Client::handle_recv ( boost::system::error_code const & ec
                             , boost::optional<fhg::com::p2p::address_t>
                             )
    {
      static sdpa::events::Codec codec;

      if (! ec)
      {
        sdpa::events::SDPAEvent::Ptr evt
          (codec.decode (std::string (m_message.data.begin(), m_message.data.end())));
        m_incoming_events.put (evt);
      }
      else if ( ec == boost::system::errc::operation_canceled
              || ec == boost::system::errc::network_down
              )
      {
        _stopping = true;
      }
      else
      {
        if (m_message.header.src != m_peer.address())
        {
          sdpa::events::ErrorEvent::Ptr
            error(new sdpa::events::ErrorEvent ( sdpa::events::ErrorEvent::SDPA_EUNKNOWN
                                               , "receiving response failed: " + boost::lexical_cast<std::string>(ec)
                                               )
                 );
          m_incoming_events.put (error);
        }
      }

      if (!_stopping)
      {
        m_peer.async_recv ( &m_message
                          , std::bind ( &Client::handle_recv
                                      , this
                                      , std::placeholders::_1
                                      , std::placeholders::_2
                                      )
                          );
      }
    }

    namespace
    {
      [[noreturn]] void handle_error_and_unexpected_event
        (sdpa::events::SDPAEvent::Ptr reply)
      {
        if ( sdpa::events::ErrorEvent *err
           = dynamic_cast<sdpa::events::ErrorEvent*> (reply.get())
           )
        {
          throw std::runtime_error
            ( "Error: reason := "
            + err->reason()
            + " code := "
            + boost::lexical_cast<std::string>(err->error_code())
            );
        }

        throw std::runtime_error
          ("Unexpected reply: " + std::string (typeid (reply.get()).name()));
      }
    }

    template<typename Expected, typename Sent>
      Expected Client::send_and_wait_for_reply (Sent event)
    {
      m_incoming_events.INDICATES_A_RACE_clear();

      static sdpa::events::Codec codec;
      m_peer.send (_drts_entrypoint_address, codec.encode (&event));

      const sdpa::events::SDPAEvent::Ptr reply (m_incoming_events.get());
      if (Expected* e = dynamic_cast<Expected*> (reply.get()))
      {
        return *e;
      }

      handle_error_and_unexpected_event (reply);
    }

    sdpa::status::code Client::wait_for_terminal_state
      (job_id_t id, job_info_t& job_info)
    {
      send_and_wait_for_reply<sdpa::events::SubscribeAckEvent>
        (sdpa::events::SubscribeEvent (id));
     sdpa::events::SDPAEvent::Ptr reply (m_incoming_events.get());

      if ( sdpa::events::JobFinishedEvent* evt
         = dynamic_cast<sdpa::events::JobFinishedEvent*> (reply.get())
         )
      {
        if (evt->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        return sdpa::status::FINISHED;
      }
      else if ( sdpa::events::JobFailedEvent* evt
              = dynamic_cast<sdpa::events::JobFailedEvent*> (reply.get())
              )
      {
        if (evt->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        job_info.error_message = evt->error_message();
        return sdpa::status::FAILED;
      }
      else if ( sdpa::events::CancelJobAckEvent* evt
              = dynamic_cast<sdpa::events::CancelJobAckEvent*> (reply.get())
              )
      {
        if (evt->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        return sdpa::status::CANCELED;
      }

      handle_error_and_unexpected_event (reply);
    }

    sdpa::status::code Client::wait_for_terminal_state_polling
      (job_id_t id, job_info_t& job_info)
    {
      sdpa::status::code state (queryJob (id, job_info));
      for (; !sdpa::status::is_terminal (state); state = queryJob (id, job_info))
      {
        static const boost::posix_time::milliseconds sleep_duration (100);
        boost::this_thread::sleep (sleep_duration);
      }
      return state;
    }

    sdpa::job_id_t Client::submitJob(const job_desc_t &desc)
    {
      return send_and_wait_for_reply<sdpa::events::SubmitJobAckEvent>
        (sdpa::events::SubmitJobEvent (boost::none, desc)).job_id();
    }

    void Client::cancelJob(const job_id_t &jid)
    {
      send_and_wait_for_reply<sdpa::events::CancelJobAckEvent>
        (sdpa::events::CancelJobEvent (jid));
    }

    sdpa::discovery_info_t Client::discoverJobStates(const we::layer::id_type& discover_id, const job_id_t &job_id)
    {
      return send_and_wait_for_reply<sdpa::events::DiscoverJobStatesReplyEvent>
        (sdpa::events::DiscoverJobStatesEvent (job_id, discover_id)).discover_result();
    }

    void Client::put_token
      (job_id_t job_id, std::string place_name, pnet::type::value::value_type value)
    {
      std::string const put_token_id
        (boost::uuids::to_string (boost::uuids::random_generator()()));

      if ( send_and_wait_for_reply<sdpa::events::put_token_ack>
           ( sdpa::events::put_token ( job_id
                                     , put_token_id
                                     , place_name
                                     , value
                                     )
           )
         .put_token_id() != put_token_id
         )
      {
        throw std::logic_error ("received put_token_ack for different put_token");
      }
    }

    sdpa::status::code Client::queryJob(const job_id_t &jid)
    {
      job_info_t info;
      return queryJob (jid, info);
    }

    sdpa::status::code Client::queryJob(const job_id_t &jid, job_info_t &info)
    {
      const sdpa::events::JobStatusReplyEvent reply
        ( send_and_wait_for_reply<sdpa::events::JobStatusReplyEvent>
          (sdpa::events::QueryJobStatusEvent (jid))
        );

      info.error_message = reply.error_message();

      return reply.status();
    }

    void Client::deleteJob(const job_id_t &jid)
    {
      send_and_wait_for_reply<sdpa::events::DeleteJobAckEvent>
        (sdpa::events::DeleteJobEvent (jid));
    }

    sdpa::client::result_t Client::retrieveResults(const job_id_t &jid)
    {
      return send_and_wait_for_reply<sdpa::events::JobResultsReplyEvent>
        (sdpa::events::RetrieveJobResultsEvent (jid)).result();
    }
  }
}
