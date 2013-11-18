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
  , name_(a_name)
  , _output_stage_name (output_stage)
  , fsm_(*this)
  , timeout_(5000U)
  , my_location_("127.0.0.1:0")
{}

Client::~Client()
{
  shutdown ();
}

void Client::perform(const seda::IEvent::Ptr &event)
{
  if (dynamic_cast<ConfigOK*>(event.get())) {
    fsm_.ConfigOk(event);
  } else if (dynamic_cast<ConfigNOK*>(event.get())) {
    fsm_.ConfigNok(event);
  } else if (StartUp * startup = dynamic_cast<StartUp*>(event.get())) {
    fsm_.Start(startup->config());
  } else if (dynamic_cast<Shutdown*>(event.get())) {
    fsm_.Shutdown();
  } else if (dynamic_cast<se::SubmitJobEvent*>(event.get())) {
    fsm_.Submit(event);
  } else if (dynamic_cast<se::SubmitJobAckEvent*>(event.get())) {
    fsm_.SubmitAck(event);
  } else if (dynamic_cast<se::SubscribeEvent*>(event.get())) {
    fsm_.Subscribe(event);
  } else if (dynamic_cast<se::SubscribeAckEvent*>(event.get())) {
    fsm_.SubscribeAck(event);
  } else if (dynamic_cast<se::QueryJobStatusEvent*>(event.get())) {
    fsm_.Query(event);
  } else if (dynamic_cast<se::JobStatusReplyEvent*>(event.get())) {
    fsm_.StatusReply(event);
  } else if (dynamic_cast<se::CancelJobEvent*>(event.get())) {
    fsm_.Cancel(event);
  } else if (dynamic_cast<se::CancelJobAckEvent*>(event.get())) {
    fsm_.CancelAck(event);
  } else if (dynamic_cast<se::RetrieveJobResultsEvent*>(event.get())) {
    fsm_.Retrieve(event);
  } else if (dynamic_cast<se::JobResultsReplyEvent*>(event.get())) {
    fsm_.Results(event);
  } else if (dynamic_cast<se::DeleteJobEvent*>(event.get())) {
    fsm_.Delete(event);
  } else if (dynamic_cast<se::DeleteJobAckEvent*>(event.get())) {
    fsm_.DeleteAck(event);
  } else if (dynamic_cast<se::ErrorEvent*>(event.get())) {
    fsm_.Error(event);
  }
  else
  {
    fsm_.Unknown(event);
  }
}

void Client::start(const config_t & config) throw (ClientException)
{
  client_stage_->send(seda::IEvent::Ptr(new StartUp(config)));

  try
  {
    seda::IEvent::Ptr reply(wait_for_reply((timeout_t)(-1)));
    if (dynamic_cast<ConfigOK*>(reply.get()))
    {
    }
    else if (ConfigNOK *cfg_nok = dynamic_cast<ConfigNOK*>(reply.get()))
    {
      throw ClientException(cfg_nok->reason());
    }
    else
    {
      throw ClientException("startup failed (got unexpected event: " + reply->str() + ")");
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("start", "this call should never timeout!");
  }
}

void Client::shutdown() throw (ClientException)
{
  if (client_stage_)
  {
    client_stage_->stop();
    seda::StageRegistry::instance().remove(client_stage_);
    client_stage_.reset();
  }
}

void Client::subscribe(const job_id_list_t& listJobIds) throw (ClientException)
{
        clear_reply();
  client_stage_->send(seda::IEvent::Ptr(new se::SubscribeEvent(name(), orchestrator_, listJobIds)));
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    if (dynamic_cast<se::SubscribeAckEvent*>(reply.get()))
    {
    }
    else if (se::ErrorEvent *err = dynamic_cast<se::ErrorEvent*>(reply.get()))
    {
      throw ClientException( "error during subscribe: reason := "+ err->reason()
                           + " code := "+ boost::lexical_cast<std::string>(err->error_code()));
    }
    else
    {
      throw ClientException("got an unexpected reply to Subscribe: " + reply->str());
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("subscribe");
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

void Client::clear_reply()
{
  if (not m_incoming_events.empty ())
  {
    m_incoming_events.clear ();
  }
}

sdpa::job_id_t Client::submitJob(const job_desc_t &desc) throw (ClientException)
{
        clear_reply();
  client_stage_->send(seda::IEvent::Ptr(new se::SubmitJobEvent(name(), orchestrator_, "", desc, "")));
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    if (se::SubmitJobAckEvent *sj_ack = dynamic_cast<se::SubmitJobAckEvent*>(reply.get()))
    {
      return sj_ack->job_id();
    }
    else if (se::ErrorEvent *err = dynamic_cast<se::ErrorEvent*>(reply.get()))
    {
      throw ClientException( "error during submit: reason := "
                           + err->reason()
                           + " code := "
                           + boost::lexical_cast<std::string>(err->error_code())
                           );
    }
    else
    {
      throw ClientException("got an unexpected reply to SubmitJob: " + reply->str());
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("submitJob");
  }
}

void Client::cancelJob(const job_id_t &jid) throw (ClientException)
{
        clear_reply();
  client_stage_->send(seda::IEvent::Ptr(new se::CancelJobEvent( name()
                                                              , orchestrator_
                                                              , jid
                                                              , "user cancel"
                                                              )
                                       )
                     );
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    if (/* se::CancelJobAckEvent *ack = */ dynamic_cast<se::CancelJobAckEvent*>(reply.get()))
    {
    }
    else if (se::ErrorEvent *err = dynamic_cast<se::ErrorEvent*>(reply.get()))
    {
      throw ClientException( "error during cancel: reason := "
                           + err->reason()
                           + " code := "
                           + boost::lexical_cast<std::string>(err->error_code())
                           );
    }
    else
    {
      throw ClientException("got an unexpected reply to CancelJob: " + reply->str());
    }
  }
  catch(const Timedout &)
  {
    throw ApiCallFailed("cancelJob");
  }
}

sdpa::status::code Client::queryJob(const job_id_t &jid) throw (ClientException)
{
  job_info_t info;
  return queryJob (jid, info);
}

sdpa::status::code Client::queryJob(const job_id_t &jid, job_info_t &info)
{
  clear_reply();
  client_stage_->send
    (seda::IEvent::Ptr(new se::QueryJobStatusEvent(name()
                                                  , orchestrator_
                                                  , jid)
                      )
    );
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    if (se::JobStatusReplyEvent *status
       = dynamic_cast<se::JobStatusReplyEvent*>(reply.get()))
    {
      info.error_code = status->error_code();
      info.error_message = status->error_message();

      return status->status();
    }
    else if (se::ErrorEvent *err
            = dynamic_cast<se::ErrorEvent*>(reply.get()))
    {
      throw ClientException( "error during query: reason := "
                           + err->reason()
                           + " code := "
                           + boost::lexical_cast<std::string>(err->error_code())
                           );
    }
    else
    {
      throw ClientException
        ("got an unexpected reply to QueryJob: " + reply->str());
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("queryJob");
  }
}

void Client::deleteJob(const job_id_t &jid) throw (ClientException)
{
        clear_reply();
  client_stage_->send(seda::IEvent::Ptr(new se::DeleteJobEvent(name(), orchestrator_, jid)));
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    if (/* se::DeleteJobAckEvent *ack = */ dynamic_cast<se::DeleteJobAckEvent*>(reply.get()))
    {
    }
    else if (se::ErrorEvent *err = dynamic_cast<se::ErrorEvent*>(reply.get()))
    {
        throw ClientException( "error during delete: reason := "
                           + err->reason()
                           + " code := "
                           + boost::lexical_cast<std::string>(err->error_code())
                           );
    }
    else
    {
      throw ClientException("got an unexpected reply to DeleteJob: " + reply->str());
    }
  }
  catch (const Timedout &)
  {
          throw ApiCallFailed("deleteJob");
  }
}

sdpa::client::result_t Client::retrieveResults(const job_id_t &jid) throw (ClientException)
{
        clear_reply();
  client_stage_->send(seda::IEvent::Ptr(new se::RetrieveJobResultsEvent(name()
                                                                      , orchestrator_
                                                                      , jid)));
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    if (se::JobResultsReplyEvent *res = dynamic_cast<se::JobResultsReplyEvent*>(reply.get()))
    {
      return res->result();
    }
    else if (se::ErrorEvent *err = dynamic_cast<se::ErrorEvent*>(reply.get()))
    {
      throw ClientException( "error during retrieve: reason := "
                           + err->reason()
                           + " code := "
                           + boost::lexical_cast<std::string>(err->error_code())
                           );
    }
    else
    {
      throw ClientException("got an unexpected reply to RetrieveResults: " + reply->str());
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("retrieveResults");
  }
}

void Client::action_configure(const config_t &cfg)
{
  if (cfg.is_set("network.timeout"))
  {
    timeout_ = cfg.get<unsigned int>("network.timeout");
  }

  if (cfg.is_set("orchestrator"))
  {
    orchestrator_ = cfg.get<std::string>("orchestrator");
  }

  if (cfg.is_set("location"))
  {
    my_location_ = cfg.get<std::string>("location");
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
    client_stage_->send(seda::IEvent::Ptr(new ConfigNOK("no orchestrator specified!")));
  }
  else
  {
    client_stage_->send(seda::IEvent::Ptr(new ConfigOK()));
  }
}

void Client::action_shutdown()
{
  _output_stage->stop();
  _output_stage.reset();
  seda::StageRegistry::instance().remove (_output_stage_name);
  action_store_reply(seda::IEvent::Ptr(new ShutdownComplete()));
}

void Client::forward_to_output_stage (const seda::IEvent::Ptr& event) const
{
  _output_stage->send (event);
}

void Client::action_store_reply(const seda::IEvent::Ptr &reply)
{
  m_incoming_events.put (reply);
}
