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

Client::ptr_t Client::create( const config_t &cfg
                            , const std::string &name_prefix
                           , const std::string &output_stage) {
  // warning: we introduce a cycle here, we have to resolve it during shutdown!
  Client::ptr_t client(new Client(name_prefix));
  seda::Stage::Ptr client_stage(new seda::Stage(name_prefix, client));
  client->setStage(client_stage);
  seda::StageRegistry::instance().insert(client_stage);
  client_stage->start();
  client->start (cfg);
  return client;
}

Client::Client(const std::string &a_name)
  : seda::Strategy(a_name)
  , timeout_(5000U)
  , _communication_thread (&Client::send_outgoing, this)
  , _stopping (false)
{}

Client::~Client()
{
  shutdown ();
}

void Client::perform(const seda::IEvent::Ptr &event)
{
  m_incoming_events.put (event);
}

namespace
{
  void kvs_error_handler (boost::system::error_code const &)
  {
    MLOG (ERROR, "could not contact KVS, terminating");
    kill (getpid (), SIGTERM);
  }
}

void Client::start(const config_t & config) throw (ClientException)
{
  if (config.is_set("network.timeout"))
  {
    timeout_ = config.get<unsigned int>("network.timeout");
  }

  if (config.is_set("orchestrator"))
  {
    orchestrator_ = config.get<std::string>("orchestrator");
  }

  m_peer.reset (new fhg::com::peer_t ( client_stage_->name()
                                     , fhg::com::host_t ("*")
                                     , fhg::com::port_t ("0")
                                     )
                   );

  _peer_thread = boost::thread (&fhg::com::peer_t::run, m_peer);
  m_peer->set_kvs_error_handler (&kvs_error_handler);
  m_peer->start ();
  m_peer->async_recv (&m_message, boost::bind(&Client::handle_recv, this, _1));

  if (orchestrator_.empty())
  {
    throw ClientException ("no orchestrator specified!");
  }
}

void Client::shutdown() throw (ClientException)
{
  _communication_thread.interrupt();
  if (_communication_thread.joinable())
  {
    _communication_thread.join();
  }

  _stopping = true;
  if (m_peer)
  {
    m_peer->stop();
  }
  if (_peer_thread.joinable())
  {
    _peer_thread.join();
  }
  m_peer.reset();

  if (client_stage_)
  {
    client_stage_->stop();
    seda::StageRegistry::instance().remove(client_stage_);
    client_stage_.reset();
  }
}

void Client::send_outgoing()
{
  //! \note No interruption point required: event_queue::get() contains one.
  for (;;)
  {
    const seda::IEvent::Ptr event (_outgoing_events.get());

    static sdpa::events::Codec codec;

    sdpa::events::SDPAEvent* sdpa_event
      (dynamic_cast<sdpa::events::SDPAEvent*>(event.get()));

    assert (sdpa_event);

    fhg::com::message_t msg;
    msg.header.dst = m_peer->resolve_name (sdpa_event->to());
    msg.header.src = m_peer->address();

    const std::string encoded_evt (codec.encode(sdpa_event));
    msg.data.assign (encoded_evt.begin(), encoded_evt.end());
    msg.header.length = msg.data.size();

    try
    {
      m_peer->async_send (&msg, boost::bind (&Client::handle_send, this, event, _1));
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

        m_peer->async_recv (&m_message, boost::bind(&Client::handle_recv, this, _1));
      }
      else if (!_stopping)
      {
        const fhg::com::p2p::address_t & addr = m_message.header.src;
        if (addr != m_peer->address())
        {
          sdpa::events::ErrorEvent::Ptr
            error(new sdpa::events::ErrorEvent ( m_peer->resolve(addr, "*unknown*")
                                               , m_peer->name()
                                               , sdpa::events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                                               , boost::lexical_cast<std::string>(ec)
                                               )
                 );
          m_incoming_events.put (error);

          m_peer->async_recv (&m_message, boost::bind(&Client::handle_recv, this, _1));
        }
      }
    }


template<typename Expected, typename Sent>
  Expected Client::send_and_wait_for_reply (Sent event)
{
  m_incoming_events.clear();
  _outgoing_events.put (event);

  const seda::IEvent::Ptr reply (wait_for_reply());
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

void Client::subscribe(const job_id_list_t& listJobIds) throw (ClientException)
{
  send_and_wait_for_reply<se::SubscribeAckEvent>
    ( seda::IEvent::Ptr ( new se::SubscribeEvent ( name()
                                                 , orchestrator_
                                                 , listJobIds
                                                 )
                        )
    );
}

sdpa::status::code Client::wait_for_terminal_state
  (job_id_t id, job_info_t& job_info)
{
  job_id_list_t listJobIds;
  listJobIds.push_back (id);
  subscribe (listJobIds);

  seda::IEvent::Ptr reply (wait_for_reply (-1));

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


seda::IEvent::Ptr Client::wait_for_reply() throw (Timedout)
{
  return wait_for_reply(timeout_);
}

// on t=0 blocks forever
seda::IEvent::Ptr Client::wait_for_reply(timeout_t t) throw (Timedout)
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
    ( seda::IEvent::Ptr ( new se::SubmitJobEvent ( name()
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
    ( seda::IEvent::Ptr ( new se::CancelJobEvent ( name()
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
      ( seda::IEvent::Ptr ( new se::QueryJobStatusEvent ( name()
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
    ( seda::IEvent::Ptr ( new se::DeleteJobEvent ( name()
                                                 , orchestrator_
                                                 , jid
                                                 )
                        )
    );
}

sdpa::client::result_t Client::retrieveResults(const job_id_t &jid) throw (ClientException)
{
  return send_and_wait_for_reply<se::JobResultsReplyEvent>
    ( seda::IEvent::Ptr ( new se::RetrieveJobResultsEvent ( name()
                                                          , orchestrator_
                                                          , jid
                                                          )
                        )
    ).result();
}
