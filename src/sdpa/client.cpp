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

#include <chrono>
#include <functional>
#include <thread>

namespace sdpa
{
  namespace client
  {
    Client::Client ( fhg::com::host_t const& orchestrator_host
                   , fhg::com::port_t const& orchestrator_port
                   , std::unique_ptr<boost::asio::io_service> peer_io_service
                   , fhg::com::Certificates const& certificates
                   )
      : _stopping (false)
      , m_peer ( std::move (peer_io_service)
               , fhg::com::host_t ("*")
               , fhg::com::port_t ("0")
               , certificates
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
      static sdpa::events::Codec const codec {};

      if (! ec)
      {
        sdpa::events::SDPAEvent::Ptr const evt
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
          sdpa::events::ErrorEvent::Ptr const
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
        if ( sdpa::events::ErrorEvent* const err
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
      std::lock_guard<std::mutex> const _ (_make_client_thread_safe);

      m_incoming_events.INDICATES_A_RACE_clear();

      static sdpa::events::Codec const codec {};
      m_peer.send (_drts_entrypoint_address, codec.encode (&event));

      const sdpa::events::SDPAEvent::Ptr reply (m_incoming_events.get());
      if (Expected* const e = dynamic_cast<Expected*> (reply.get()))
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
     sdpa::events::SDPAEvent::Ptr const reply (m_incoming_events.get());

      if ( sdpa::events::JobFinishedEvent* const job_finished
         = dynamic_cast<sdpa::events::JobFinishedEvent*> (reply.get())
         )
      {
        if (job_finished->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        return sdpa::status::FINISHED;
      }
      else if ( sdpa::events::JobFailedEvent* const job_failed
              = dynamic_cast<sdpa::events::JobFailedEvent*> (reply.get())
              )
      {
        if (job_failed->job_id() != id)
        {
          throw std::runtime_error ("got status change for different job");
        }
        job_info.error_message = job_failed->error_message();
        return sdpa::status::FAILED;
      }
      else if ( sdpa::events::CancelJobAckEvent* const cancel_ack
              = dynamic_cast<sdpa::events::CancelJobAckEvent*> (reply.get())
              )
      {
        if (cancel_ack->job_id() != id)
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
      while (!sdpa::status::is_terminal (state))
      {
        std::this_thread::sleep_for (std::chrono::milliseconds (100));

        state = queryJob (id, job_info);
      }
      return state;
    }

    sdpa::job_id_t Client::submitJob (we::type::activity_t const& activity)
    {
      return send_and_wait_for_reply<sdpa::events::SubmitJobAckEvent>
        (sdpa::events::SubmitJobEvent (boost::none, activity)).job_id();
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

      sdpa::events::put_token_response const response
        ( send_and_wait_for_reply<sdpa::events::put_token_response>
            ( sdpa::events::put_token ( job_id
                                      , put_token_id
                                      , place_name
                                      , value
                                      )
            )
        );

      if (response.put_token_id() != put_token_id)
      {
        throw std::logic_error ("received put_token_response for different put_token");
      }

      return response.get();
    }

    pnet::type::value::value_type Client::workflow_response
      (job_id_t job_id, std::string place_name, pnet::type::value::value_type value)
    {
      std::string const workflow_response_id
        (boost::uuids::to_string (boost::uuids::random_generator()()));

      sdpa::events::workflow_response_response const response
        ( send_and_wait_for_reply<sdpa::events::workflow_response_response>
            ( sdpa::events::workflow_response ( job_id
                                              , workflow_response_id
                                              , place_name
                                              , value
                                              )
            )
        );

      if (response.workflow_response_id() != workflow_response_id)
      {
        throw std::logic_error
          ("received workflow_response_response for different workflow_response");
      }

      return response.get();
    }

    sdpa::status::code Client::queryJob(const job_id_t &jid)
    {
      job_info_t UNUSED_info;
      return queryJob (jid, UNUSED_info);
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

    we::type::activity_t Client::retrieveResults(const job_id_t &jid)
    {
      return send_and_wait_for_reply<sdpa::events::JobResultsReplyEvent>
        (sdpa::events::RetrieveJobResultsEvent (jid)).result();
    }
  }
}
