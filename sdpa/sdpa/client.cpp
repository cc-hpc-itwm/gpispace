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

#include <fhg/util/macros.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/uuid/random_generator.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace sdpa
{
  namespace client
  {
    namespace
    {
      void kvs_error_handler (boost::system::error_code const &)
      {
        throw std::runtime_error ("could not contact KVS, terminating");
      }
    }

    Client::Client (std::string orchestrator)
      : _name ("gspcc-" + boost::uuids::to_string (boost::uuids::random_generator()()))
      , orchestrator_ (orchestrator)
      , m_peer (_name, fhg::com::host_t ("*"), fhg::com::port_t ("0"), fhg::com::kvs::global_kvs())
      , _peer_thread (&fhg::com::peer_t::run, &m_peer)
      , _stopping (false)
    {
      m_peer.set_kvs_error_handler (&kvs_error_handler);
      m_peer.start ();
      m_peer.async_recv (&m_message, boost::bind(&Client::handle_recv, this, _1));
    }

    Client::~Client()
    {
      _stopping = true;
      m_peer.stop();
      if (_peer_thread.joinable())
      {
        _peer_thread.join();
      }
    }

    fhg::com::message_t Client::message_for_event
      (const sdpa::events::SDPAEvent* event)
    {
      static sdpa::events::Codec codec;

      const std::string encoded_evt (codec.encode (event));
      fhg::com::message_t msg (encoded_evt.begin(), encoded_evt.end());
      msg.header.dst = m_peer.resolve_name (event->to());
      msg.header.src = m_peer.address();
      msg.header.length = msg.data.size();

      return msg;
    }

    void Client::handle_recv (boost::system::error_code const & ec)
    {
      static sdpa::events::Codec codec;

      if (! ec)
      {
        sdpa::events::SDPAEvent::Ptr evt
          (codec.decode (std::string (m_message.data.begin(), m_message.data.end())));
        m_incoming_events.put (evt);
      }
      else
      {
        const fhg::com::p2p::address_t & addr = m_message.header.src;
        if (addr != m_peer.address())
        {
          sdpa::events::ErrorEvent::Ptr
            error(new sdpa::events::ErrorEvent ( m_peer.resolve(addr, "*unknown*")
                                               , m_peer.name()
                                               , sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                                               , boost::lexical_cast<std::string>(ec)
                                               )
                 );
          m_incoming_events.put (error);
        }
      }

      if (!_stopping)
      {
        m_peer.async_recv (&m_message, boost::bind(&Client::handle_recv, this, _1));
      }
    }

    namespace
    {
      FHG_ATTRIBUTE_NORETURN
        void handle_bad_event (sdpa::events::SDPAEvent::Ptr reply)
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

        throw std::runtime_error ("Unexpected reply: " + reply->str());
      }
    }

    template<typename Expected, typename Sent>
      Expected Client::send_and_wait_for_reply (Sent event)
    {
      m_incoming_events.clear();

      fhg::com::message_t msg (message_for_event (&event));

      m_peer.send (&msg);

      const sdpa::events::SDPAEvent::Ptr reply (m_incoming_events.get());
      if (Expected* e = dynamic_cast<Expected*> (reply.get()))
      {
        return *e;
      }

      handle_bad_event (reply);
    }

    sdpa::status::code Client::wait_for_terminal_state
      (job_id_t id, job_info_t& job_info)
    {
      send_and_wait_for_reply<sdpa::events::SubscribeAckEvent>
        (sdpa::events::SubscribeEvent (_name, orchestrator_, job_id_list_t (1, id)));
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
        job_info.error_code = evt->error_code();
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

      handle_bad_event (reply);
    }

    sdpa::status::code Client::wait_for_terminal_state_polling
      (job_id_t id, job_info_t&)
    {
      sdpa::status::code state (queryJob (id));
      for (; !sdpa::status::is_terminal (state); state = queryJob (id))
      {
        static const boost::posix_time::milliseconds sleep_duration (100);
        boost::this_thread::sleep (sleep_duration);
      }
      return state;
    }

    sdpa::job_id_t Client::submitJob(const job_desc_t &desc)
    {
      return send_and_wait_for_reply<sdpa::events::SubmitJobAckEvent>
        (sdpa::events::SubmitJobEvent (_name, orchestrator_, boost::none, desc)).job_id();
    }

    void Client::cancelJob(const job_id_t &jid)
    {
      send_and_wait_for_reply<sdpa::events::CancelJobAckEvent>
        (sdpa::events::CancelJobEvent (_name, orchestrator_, jid));
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
        (sdpa::events::QueryJobStatusEvent (_name, orchestrator_, jid))
        );

      info.error_code = reply.error_code();
      info.error_message = reply.error_message();

      return reply.status();
    }

    void Client::deleteJob(const job_id_t &jid)
    {
      send_and_wait_for_reply<sdpa::events::DeleteJobAckEvent>
        (sdpa::events::DeleteJobEvent (_name, orchestrator_, jid));
    }

    sdpa::client::result_t Client::retrieveResults(const job_id_t &jid)
    {
      return send_and_wait_for_reply<sdpa::events::JobResultsReplyEvent>
        (sdpa::events::RetrieveJobResultsEvent (_name, orchestrator_, jid)).result();
    }
  }
}
