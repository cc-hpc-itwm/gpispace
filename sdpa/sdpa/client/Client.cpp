#include "Client.hpp"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <seda/StageRegistry.hpp>
#include <sdpa/job_states.hpp>

#include <sdpa/client/ClientEvents.hpp>

#include <sdpa/events/SubmitJobEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/SubscribeEvent.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>

#include <sdpa/events/CodecStrategy.hpp>

#include <sdpa/com/NetworkStrategy.hpp>

namespace se = sdpa::events;
using namespace sdpa::client;

namespace
{
  void kvs_error_handler (boost::system::error_code const &)
  {
    MLOG (ERROR, "could not contact KVS, terminating");
    kill (getpid (), SIGTERM);
  }
}

Client::Client (const config_t& config, const std::string &a_name)
  : _name (a_name)
  , timeout_ ( config.is_set("network.timeout")
             ? config.get<unsigned int>("network.timeout")
             : 5000U
             )
  , orchestrator_ ( config.is_set("orchestrator")
                  ? config.get<std::string>("orchestrator")
                  : throw ClientException ("no orchestrator specified!")
                  )
  , _communication_thread (&Client::send_outgoing, this)
  , m_peer (_name, fhg::com::host_t ("*"), fhg::com::port_t ("0"))
  , _peer_thread (&fhg::com::peer_t::run, &m_peer)
  , _stopping (false)
{
  m_peer.set_kvs_error_handler (&kvs_error_handler);
  m_peer.start ();
  m_peer.async_recv (&m_message, boost::bind(&Client::handle_recv, this, _1));
}

Client::~Client()
{
  _communication_thread.interrupt();
  if (_communication_thread.joinable())
  {
    _communication_thread.join();
  }

  _stopping = true;
  m_peer.stop();
  if (_peer_thread.joinable())
  {
    _peer_thread.join();
  }
}

fhg::com::message_t Client::message_for_event (sdpa::events::SDPAEvent* event)
{
  static sdpa::events::Codec codec;

  fhg::com::message_t msg;
  msg.header.dst = m_peer.resolve_name (event->to());
  msg.header.src = m_peer.address();

  const std::string encoded_evt (codec.encode (event));
  msg.data.assign (encoded_evt.begin(), encoded_evt.end());
  msg.header.length = msg.data.size();

  return msg;
}

void Client::send_outgoing()
{
  //! \note No interruption point required: event_queue::get() contains one.
  for (;;)
  {
    const sdpa::events::SDPAEvent::Ptr event (_outgoing_events.get());

    sdpa::events::SDPAEvent* sdpa_event
      (dynamic_cast<sdpa::events::SDPAEvent*>(event.get()));
    assert (sdpa_event);

    fhg::com::message_t msg (message_for_event (sdpa_event));;

    try
    {
      m_peer.async_send (&msg, boost::bind (&Client::handle_send, this, event, _1));
    }
    catch (std::exception const & ex)
    {
      sdpa::events::ErrorEvent::Ptr ptrErrEvt
        (new sdpa::events::ErrorEvent( sdpa_event->to()
                                     , sdpa_event->from()
                                     , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                     , sdpa_event->str())
        );
      m_incoming_events.put (ptrErrEvt);
    }
  }
}

void Client::handle_send ( seda::IEvent::Ptr event
                         , boost::system::error_code const & ec
                         )
{
  if (ec)
  {
    sdpa::events::SDPAEvent* e
      (dynamic_cast<sdpa::events::SDPAEvent*>(event.get()));

    assert (e);

    DMLOG ( WARN
          , "send failed:"
          << " ec := " << ec
          << " msg := " << ec.message ()
          << " event := " << e->str()
          << " to := " << e->to ()
          << " from := " << e->from ()
          );

    sdpa::events::ErrorEvent::Ptr ptrErrEvt
      (new sdpa::events::ErrorEvent( e->to()
                                   , e->from()
                                   , sdpa::events::ErrorEvent::SDPA_ENETWORKFAILURE
                                   , e->str())
      );
    m_incoming_events.put (ptrErrEvt);
  }
}

    void Client::handle_recv (boost::system::error_code const & ec)
    {
      static sdpa::events::Codec codec;

      if (! ec)
      {
        // convert m_message to event
        try
        {
          sdpa::events::SDPAEvent::Ptr evt
            (codec.decode (std::string (m_message.data.begin(), m_message.data.end())));
          DLOG(TRACE, "received event: " << evt->str());
          m_incoming_events.put (evt);
        }
        catch (std::exception const & ex)
        {
          LOG(WARN, "could not handle incoming message: " << ex.what());
        }

        m_peer.async_recv (&m_message, boost::bind(&Client::handle_recv, this, _1));
      }
      else if (!_stopping)
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

          m_peer.async_recv (&m_message, boost::bind(&Client::handle_recv, this, _1));
        }
      }
    }


template<typename Expected, typename Sent>
  Expected Client::send_and_wait_for_reply (Sent event)
{
  m_incoming_events.clear();
  _outgoing_events.put (event);

  const sdpa::events::SDPAEvent::Ptr reply (wait_for_reply());
  if (Expected* e = dynamic_cast<Expected*> (reply.get()))
  {
    return *e;
  }
  else if (se::ErrorEvent* err = dynamic_cast<se::ErrorEvent*> (reply.get()))
  {
    throw ClientException ( "Error: reason := "+ err->reason()
                          + " code := "
                          + boost::lexical_cast<std::string> (err->error_code())
                          );
  }
  else
  {
    throw ClientException ("Unexpected reply: " + reply->str());
  }
}

sdpa::status::code Client::wait_for_terminal_state
  (job_id_t id, job_info_t& job_info)
{
  send_and_wait_for_reply<se::SubscribeAckEvent>
    ( sdpa::events::SDPAEvent::Ptr ( new se::SubscribeEvent ( _name
                                                 , orchestrator_
                                                 , job_id_list_t (1, id)
                                                 )
                        )
    );

  sdpa::events::SDPAEvent::Ptr reply (wait_for_reply (-1));

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
  else if ( sdpa::events::ErrorEvent *err
          = dynamic_cast<sdpa::events::ErrorEvent*>(reply.get())
          )
  {
    throw std::runtime_error
      ( "got error event: reason := "
      + err->reason()
      + " code := "
      + boost::lexical_cast<std::string>(err->error_code())
      );
  }
  else
  {
    throw std::runtime_error
      (std::string ("unexpected reply: ") + (reply ? reply->str() : "null"));
  }
}


sdpa::events::SDPAEvent::Ptr Client::wait_for_reply() throw (Timedout)
{
  return wait_for_reply(timeout_);
}

// on t=0 blocks forever
sdpa::events::SDPAEvent::Ptr Client::wait_for_reply(timeout_t t) throw (Timedout)
{
  if (t == (timeout_t)(-1))
  {
    return m_incoming_events.get ();
  }
  else
  {
    try
    {
      return m_incoming_events.get (boost::posix_time::milliseconds (t));
    }
    catch (fhg::thread::operation_timedout const &)
    {
      throw Timedout ("did not receive reply");
    }
  }
}

sdpa::job_id_t Client::submitJob(const job_desc_t &desc) throw (ClientException)
{
  return send_and_wait_for_reply<se::SubmitJobAckEvent>
    ( sdpa::events::SDPAEvent::Ptr ( new se::SubmitJobEvent ( _name
                                                 , orchestrator_
                                                 , ""
                                                 , desc
                                                 , ""
                                                 )
                        )
    ).job_id();
}

void Client::cancelJob(const job_id_t &jid) throw (ClientException)
{
  send_and_wait_for_reply<se::CancelJobAckEvent>
    ( sdpa::events::SDPAEvent::Ptr ( new se::CancelJobEvent ( _name
                                                 , orchestrator_
                                                 , jid
                                                 , "user cancel"
                                                 )
                        )
    );
}

sdpa::status::code Client::queryJob(const job_id_t &jid) throw (ClientException)
{
  job_info_t info;
  return queryJob (jid, info);
}

sdpa::status::code Client::queryJob(const job_id_t &jid, job_info_t &info)
{
  const se::JobStatusReplyEvent reply
    ( send_and_wait_for_reply<se::JobStatusReplyEvent>
      ( sdpa::events::SDPAEvent::Ptr ( new se::QueryJobStatusEvent ( _name
                                                        , orchestrator_
                                                        , jid
                                                        )
                          )
      )
    );

  info.error_code = reply.error_code();
  info.error_message = reply.error_message();

  return reply.status();
}

void Client::deleteJob(const job_id_t &jid) throw (ClientException)
{
  send_and_wait_for_reply<se::DeleteJobAckEvent>
    ( sdpa::events::SDPAEvent::Ptr ( new se::DeleteJobEvent ( _name
                                                 , orchestrator_
                                                 , jid
                                                 )
                        )
    );
}

sdpa::client::result_t Client::retrieveResults(const job_id_t &jid) throw (ClientException)
{
  return send_and_wait_for_reply<se::JobResultsReplyEvent>
    ( sdpa::events::SDPAEvent::Ptr ( new se::RetrieveJobResultsEvent ( _name
                                                          , orchestrator_
                                                          , jid
                                                          )
                        )
    ).result();
}
