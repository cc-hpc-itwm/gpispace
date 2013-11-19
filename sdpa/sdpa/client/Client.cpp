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
  Client::ptr_t client(new Client(name_prefix, output_stage));
  seda::Stage::Ptr client_stage(new seda::Stage(name_prefix, client));
  client->setStage(client_stage);
  seda::StageRegistry::instance().insert(client_stage);
  client_stage->start();
  client->start (cfg);
  return client;
}

Client::Client(const std::string &a_name, const std::string &output_stage)
  : seda::Strategy(a_name)
  , _output_stage_name (output_stage)
  , timeout_(5000U)
  , _communication_thread (&Client::send_outgoing, this)
{}

Client::~Client()
{
  shutdown ();
}

void Client::perform(const seda::IEvent::Ptr &event)
{
  m_incoming_events.put (event);
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

  sdpa::com::NetworkStrategy::ptr_t net
    (new sdpa::com::NetworkStrategy( client_stage_->name()
                                   , client_stage_->name() //"sdpac" // TODO encode user, pid, etc
                                   , fhg::com::host_t ("*")
                                   , fhg::com::port_t ("0")
                                   )
    );
  _output_stage = seda::Stage::Ptr (new seda::Stage (_output_stage_name, net));
  seda::StageRegistry::instance().insert (_output_stage);
  _output_stage->start();

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

  if (_output_stage)
  {
    _output_stage->stop();
    seda::StageRegistry::instance().remove (_output_stage_name);
    _output_stage.reset();
  }

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
    _output_stage->send (event);
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
