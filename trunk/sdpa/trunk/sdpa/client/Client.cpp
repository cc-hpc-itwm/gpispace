#include "Client.hpp"

#include <seda/StageRegistry.hpp>
#include <sdpa/LoggingConfigurator.hpp>
#include <sdpa/client/ConfigEvents.hpp>

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

namespace se = sdpa::events;
using namespace sdpa::client;

Client::ptr_t Client::create(const std::string &name_prefix, const std::string &output_stage) {
  // warning: we introduce a cycle here, we have to resolve it during shutdown!
  Client::ptr_t client(new Client(name_prefix, output_stage));
  seda::Stage::Ptr client_stage(new seda::Stage(name_prefix, client));
  client->setStage(client_stage);
  seda::StageRegistry::instance().insert(client_stage);
  client_stage->start();
  return client;
}

Client::~Client()
{
  SDPA_LOG_DEBUG("destroying client api");
}

void Client::perform(const seda::IEvent::Ptr &event)
{
  SDPA_LOG_DEBUG("got event: " << event->str());
  if (dynamic_cast<ConfigOK*>(event.get())) {
    SDPA_LOG_DEBUG("ok");
    fsm_.ConfigOk(event);
  } else if (dynamic_cast<ConfigNOK*>(event.get())) {
    SDPA_LOG_DEBUG("nok");
    fsm_.ConfigNok(event);
  } else if (StartUp * start = dynamic_cast<StartUp*>(event.get())) {
    SDPA_LOG_DEBUG("start");
    fsm_.Start(start->config());
  } else if (Shutdown *shut = dynamic_cast<Shutdown*>(event.get())) {
    SDPA_LOG_DEBUG("shut");
    fsm_.Shutdown();
  } else if (dynamic_cast<se::SubmitJobEvent*>(event.get())) {
    SDPA_LOG_DEBUG("sub");
    fsm_.Submit(event);
  } else if (dynamic_cast<se::SubmitJobAckEvent*>(event.get())) {
    SDPA_LOG_DEBUG("ack");
    fsm_.SubmitAck(event);
  } else if (dynamic_cast<se::QueryJobStatusEvent*>(event.get())) {
    SDPA_LOG_DEBUG("qstat");
    fsm_.Query(event);
  } else if (dynamic_cast<se::JobStatusReplyEvent*>(event.get())) {
    SDPA_LOG_DEBUG("rstat");
    fsm_.StatusReply(event);
  } else if (dynamic_cast<se::CancelJobEvent*>(event.get())) {
    SDPA_LOG_DEBUG("kill");
    fsm_.Cancel(event);
  } else if (dynamic_cast<se::CancelJobAckEvent*>(event.get())) {
    SDPA_LOG_DEBUG("ack");
    fsm_.CancelAck(event);
  } else if (dynamic_cast<se::RetrieveJobResultsEvent*>(event.get())) {
    SDPA_LOG_DEBUG("get");
    fsm_.Retrieve(event);
  } else if (dynamic_cast<se::JobResultsReplyEvent*>(event.get())) {
    SDPA_LOG_DEBUG("get");
    fsm_.Results(event);
  } else if (dynamic_cast<se::DeleteJobEvent*>(event.get())) {
    SDPA_LOG_DEBUG("del");
    fsm_.Delete(event);
  } else if (dynamic_cast<se::DeleteJobAckEvent*>(event.get())) {
    SDPA_LOG_DEBUG("ack");
    fsm_.DeleteAck(event);
  }
}

void Client::start(const Client::config_t & config)
{
  std::clog << "starting up" << std::endl;

  client_stage_->send(seda::IEvent::Ptr(new StartUp(config)));

  std::clog << "waiting until configuration is done" << std::endl;
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (dynamic_cast<ConfigOK*>(reply.get()))
  {
    SDPA_LOG_INFO("configuration was ok");
  }
  else if (dynamic_cast<ConfigNOK*>(reply.get()))
  {
    throw std::runtime_error("configuration was not ok");
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

void Client::shutdown()
{
  client_stage_->send(seda::IEvent::Ptr(new Shutdown()));
  wait_for_reply();

  client_stage_->stop();
  seda::StageRegistry::instance().remove(client_stage_);
  client_stage_.reset();
}

seda::IEvent::Ptr Client::wait_for_reply(const timeout_t timeout)
{
  boost::unique_lock<boost::mutex> lock(mtx_);
  while (reply_.get() == NULL)
  {
    cond_.wait(lock);
  }
  seda::IEvent::Ptr ret(reply_);
  reply_.reset();
  return ret;
}

sdpa::job_id_t Client::submitJob(const job_desc_t &desc)
{
  SDPA_LOG_DEBUG("submitting job with description = " << desc);
  client_stage_->send(seda::IEvent::Ptr(new se::SubmitJobEvent(name(), /* config.get("sdpa.topology.orchestrator") */ "orchestrator", desc)));
  SDPA_LOG_DEBUG("waiting for a reply");
  // TODO: wait_for_reply(config.get<timeout_t>("sdpa.network.timeout")
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::SubmitJobAckEvent *sj_ack = dynamic_cast<se::SubmitJobAckEvent*>(reply.get()))
  {
    SDPA_LOG_DEBUG("got an acknowledge: "
        << sj_ack->from()
        << " -> "
        << sj_ack->to()
        << " job_id: "
        << sj_ack->job_id());
    return sj_ack->job_id();
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

void Client::cancelJob(const job_id_t &jid)
{
  SDPA_LOG_DEBUG("cancelling job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::CancelJobEvent(name()
                                                             , "orchestrator"
                                                             , jid)));
  SDPA_LOG_DEBUG("waiting for a reply");
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::CancelJobAckEvent *ack = dynamic_cast<se::CancelJobAckEvent*>(reply.get()))
  {
    SDPA_LOG_DEBUG("cancellation has been acknowledged");
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

int Client::queryJob(const job_id_t &jid)
{
  SDPA_LOG_DEBUG("querying status of job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::QueryJobStatusEvent(name()
                                                                 , "orchestrator"
                                                                 , jid)));
  SDPA_LOG_DEBUG("waiting for a reply");
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::JobStatusReplyEvent *status = dynamic_cast<se::JobStatusReplyEvent*>(reply.get()))
  {
    SDPA_LOG_DEBUG("got status for " << status->job_id() << ": " << status->status());
    return status->status();
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

void Client::deleteJob(const job_id_t &jid)
{
  SDPA_LOG_DEBUG("deleting job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::DeleteJobEvent(name()
                                                             , "orchestrator"
                                                             , jid)));
  SDPA_LOG_DEBUG("waiting for a reply");
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::DeleteJobAckEvent *ack = dynamic_cast<se::DeleteJobAckEvent*>(reply.get()))
  {
    SDPA_LOG_DEBUG("deletion of job has been acknowledged");
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

sdpa::client::Client::result_t Client::retrieveResults(const job_id_t &jid)
{
  SDPA_LOG_DEBUG("retrieving results of job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::RetrieveJobResultsEvent(name()
                                                                      , "orchestrator"
                                                                      , jid)));
  SDPA_LOG_DEBUG("waiting for a reply");
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::JobResultsReplyEvent *res = dynamic_cast<se::JobResultsReplyEvent*>(reply.get()))
  {
    SDPA_LOG_DEBUG("results: " << res->result());
    return res->result();
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

void Client::action_configure(const Client::config_t &cfg)
{
  // configure logging according to config
  sdpa::logging::Configurator::configure(/*use default config for now*/);  
  SDPA_LOG_DEBUG("configuring my environment");
  // send event to myself
  if (cfg == "config-nok") client_stage_->send(seda::IEvent::Ptr(new ConfigNOK()));
  else client_stage_->send(seda::IEvent::Ptr(new ConfigOK()));
}

void Client::action_config_ok()
{
  SDPA_LOG_DEBUG("config ok");
}

void Client::action_config_nok()
{
  SDPA_LOG_DEBUG("config not ok");
}

void Client::action_shutdown()
{
  SDPA_LOG_DEBUG("shutting down");

  SDPA_LOG_DEBUG("waking up api");
  action_store_reply(seda::IEvent::Ptr(new ShutdownComplete()));
}

void Client::action_submit(const seda::IEvent::Ptr &e)
{
  SDPA_LOG_DEBUG("sending submit message <TODO>");
//  seda::StageRegistry::instance().lookup(output_stage_)->send(e);

  SDPA_LOG_WARN("faking acknowledgement");
  client_stage_->send(seda::IEvent::Ptr(new se::SubmitJobAckEvent("orchestrator", name(), job_id_t())));
}

void Client::action_cancel(const seda::IEvent::Ptr &e)
{
  SDPA_LOG_DEBUG("sending cancel message");

  SDPA_LOG_WARN("faking acknowledgement");
  if (se::CancelJobEvent *cancel = dynamic_cast<se::CancelJobEvent*>(e.get()))
  {
    client_stage_->send(seda::IEvent::Ptr(new se::CancelJobAckEvent("orchestrator"
                                                                   , name()
                                                                   , cancel->job_id())));
  }
  else
  {
    SDPA_LOG_ERROR("i got some very strange event: " << e->str());
  }
}

void Client::action_query(const seda::IEvent::Ptr &e)
{
  SDPA_LOG_DEBUG("sending query message");
//  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
  SDPA_LOG_WARN("faking status reply");
  if (se::QueryJobStatusEvent* q = dynamic_cast<se::QueryJobStatusEvent*>(e.get()))
  {
    client_stage_->send(seda::IEvent::Ptr(new se::JobStatusReplyEvent("orchestrator"
                                                                    , name()
                                                                    , q->job_id()
                                                                    , 42)));
  }
  else
  {
    SDPA_LOG_ERROR("i got some very strange event: " << e->str());
  }
}

void Client::action_retrieve(const seda::IEvent::Ptr &e)
{
  SDPA_LOG_DEBUG("sending retrieve message");
  SDPA_LOG_WARN("faking results");
  if (se::RetrieveJobResultsEvent *retr = dynamic_cast<se::RetrieveJobResultsEvent*>(e.get()))
  {
    client_stage_->send(seda::IEvent::Ptr(new se::JobResultsReplyEvent("orchestrator"
                                                                     , name()
                                                                     , retr->job_id()
                                                                     , "dummy results")));
  }
  else
  {
    SDPA_LOG_ERROR("i got some very strange event: " << e->str());
  }
}

void Client::action_delete(const seda::IEvent::Ptr &e)
{
  SDPA_LOG_DEBUG("sending delete message");
  SDPA_LOG_WARN("faking acknowledgement");
  if (se::DeleteJobEvent *del = dynamic_cast<se::DeleteJobEvent*>(e.get()))
  {
    client_stage_->send(seda::IEvent::Ptr(new se::DeleteJobAckEvent("orchestrator"
                                                                   , name())));
  }
  else
  {
    SDPA_LOG_ERROR("i got some very strange event: " << e->str());
  }
}

void Client::action_store_reply(const seda::IEvent::Ptr &reply)
{
  boost::unique_lock<boost::mutex> lock(mtx_);

  SDPA_LOG_DEBUG("storing reply message: " << reply->str());
  reply_ = reply;
  cond_.notify_one();
}
