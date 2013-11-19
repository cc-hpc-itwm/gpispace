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

void Client::subscribe(const job_id_list_t& listJobIds) throw (ClientException)
{
        clear_reply();
  _output_stage->send(seda::IEvent::Ptr(new se::SubscribeEvent(name(), orchestrator_, listJobIds)));
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
  _output_stage->send(seda::IEvent::Ptr(new se::SubmitJobEvent(name(), orchestrator_, "", desc, "")));
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
  _output_stage->send(seda::IEvent::Ptr(new se::CancelJobEvent( name()
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
  _output_stage->send
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
  _output_stage->send(seda::IEvent::Ptr(new se::DeleteJobEvent(name(), orchestrator_, jid)));
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
  _output_stage->send(seda::IEvent::Ptr(new se::RetrieveJobResultsEvent(name()
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
