#include "Client.hpp"

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <seda/StageRegistry.hpp>
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

#include "ServerMock.hpp"

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
  MLOG(DEBUG, "destroying client api");
}

void Client::perform(const seda::IEvent::Ptr &event)
{
  DMLOG(DEBUG, "got event: " << event->str());
  if (dynamic_cast<ConfigOK*>(event.get())) {
    DMLOG(DEBUG, "ok");
    fsm_.ConfigOk(event);
  } else if (dynamic_cast<ConfigNOK*>(event.get())) {
    DMLOG(DEBUG,"nok");
    fsm_.ConfigNok(event);
  } else if (StartUp * start = dynamic_cast<StartUp*>(event.get())) {
    DMLOG(DEBUG,"start");
    fsm_.Start(start->config());
  } else if (Shutdown *shut = dynamic_cast<Shutdown*>(event.get())) {
    DMLOG(DEBUG,"shut");
    fsm_.Shutdown();
  } else if (dynamic_cast<se::SubmitJobEvent*>(event.get())) {
    DMLOG(DEBUG,"sub");
    fsm_.Submit(event);
  } else if (dynamic_cast<se::SubmitJobAckEvent*>(event.get())) {
    DMLOG(DEBUG,"ack");
    fsm_.SubmitAck(event);
  } else if (dynamic_cast<se::QueryJobStatusEvent*>(event.get())) {
    DMLOG(DEBUG,"qstat");
    fsm_.Query(event);
  } else if (dynamic_cast<se::JobStatusReplyEvent*>(event.get())) {
    DMLOG(DEBUG,"rstat");
    fsm_.StatusReply(event);
  } else if (dynamic_cast<se::CancelJobEvent*>(event.get())) {
    DMLOG(DEBUG,"kill");
    fsm_.Cancel(event);
  } else if (dynamic_cast<se::CancelJobAckEvent*>(event.get())) {
    DMLOG(DEBUG,"ack");
    fsm_.CancelAck(event);
  } else if (dynamic_cast<se::RetrieveJobResultsEvent*>(event.get())) {
    DMLOG(DEBUG,"get");
    fsm_.Retrieve(event);
  } else if (dynamic_cast<se::JobResultsReplyEvent*>(event.get())) {
    DMLOG(DEBUG,"get");
    fsm_.Results(event);
  } else if (dynamic_cast<se::DeleteJobEvent*>(event.get())) {
    DMLOG(DEBUG,"del");
    fsm_.Delete(event);
  } else if (dynamic_cast<se::DeleteJobAckEvent*>(event.get())) {
    DMLOG(DEBUG,"ack");
    fsm_.DeleteAck(event);
  }
}

void Client::start(const Client::config_t & config)
{
  std::clog << "I: starting up" << std::endl;
  fhg::log::Configurator::configure(/*use default config for now*/);

  client_stage_->send(seda::IEvent::Ptr(new StartUp(config)));

  std::clog << "waiting until configuration is done" << std::endl;
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (dynamic_cast<ConfigOK*>(reply.get()))
  {
    MLOG(INFO,"configuration was ok");
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
  MLOG(INFO,"submitting job with description = " << desc);
  client_stage_->send(seda::IEvent::Ptr(new se::SubmitJobEvent(name(), /* config.get("sdpa.topology.orchestrator") */ "orchestrator", desc)));
  DMLOG(DEBUG,"waiting for a reply");
  // TODO: wait_for_reply(config.get<timeout_t>("sdpa.network.timeout")
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::SubmitJobAckEvent *sj_ack = dynamic_cast<se::SubmitJobAckEvent*>(reply.get()))
  {
    DMLOG(DEBUG,"got an acknowledge: "
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
  MLOG(INFO,"cancelling job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::CancelJobEvent(name()
                                                             , "orchestrator"
                                                             , jid)));
  DMLOG(DEBUG,"waiting for a reply");
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::CancelJobAckEvent *ack = dynamic_cast<se::CancelJobAckEvent*>(reply.get()))
  {
    DMLOG(DEBUG,"cancellation has been acknowledged");
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

int Client::queryJob(const job_id_t &jid)
{
  MLOG(INFO,"querying status of job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::QueryJobStatusEvent(name()
                                                                 , "orchestrator"
                                                                 , jid)));
  DMLOG(DEBUG,"waiting for a reply");
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::JobStatusReplyEvent *status = dynamic_cast<se::JobStatusReplyEvent*>(reply.get()))
  {
    DMLOG(DEBUG,"got status for " << status->job_id() << ": " << status->status());
    return status->status();
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

void Client::deleteJob(const job_id_t &jid)
{
  MLOG(INFO,"deleting job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::DeleteJobEvent(name()
                                                             , "orchestrator"
                                                             , jid)));
  DMLOG(DEBUG,"waiting for a reply");
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::DeleteJobAckEvent *ack = dynamic_cast<se::DeleteJobAckEvent*>(reply.get()))
  {
    DMLOG(DEBUG,"deletion of job has been acknowledged");
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

sdpa::client::Client::result_t Client::retrieveResults(const job_id_t &jid)
{
  MLOG(INFO,"retrieving results of job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::RetrieveJobResultsEvent(name()
                                                                      , "orchestrator"
                                                                      , jid)));
  DMLOG(DEBUG,"waiting for a reply");
  seda::IEvent::Ptr reply(wait_for_reply());
  // check event type
  if (se::JobResultsReplyEvent *res = dynamic_cast<se::JobResultsReplyEvent*>(reply.get()))
  {
    DMLOG(DEBUG,"results: " << res->result());
    return res->result();
  }
  else
  {
    throw std::runtime_error("timedout/failed");
  }
}

void Client::action_configure(const Client::config_t &cfg)
{
  MLOG(INFO, "configuring my environment");
  // configure logging according to config
  //   TODO: do something

  DMLOG(DEBUG, "creating server mock stage");
  {
    seda::Strategy::Ptr server_strat(new ServerMock(name()));
    seda::Stage::Ptr server(new seda::Stage(output_stage_, server_strat));
    seda::StageRegistry::instance().insert(server);
    server->start();
  }

  // send event to myself
  if (cfg == "config-nok") client_stage_->send(seda::IEvent::Ptr(new ConfigNOK()));
  else client_stage_->send(seda::IEvent::Ptr(new ConfigOK()));
}

void Client::action_config_ok()
{
  DMLOG(DEBUG,"config ok");
}

void Client::action_config_nok()
{
  DMLOG(DEBUG,"config not ok");
}

void Client::action_shutdown()
{
  MLOG(INFO,"shutting down");

  DMLOG(DEBUG, "removing server mock");
  {
    seda::StageRegistry::instance().lookup(output_stage_)->stop();
    seda::StageRegistry::instance().remove(output_stage_);
  }

  DMLOG(DEBUG,"waking up api");
  action_store_reply(seda::IEvent::Ptr(new ShutdownComplete()));
}

void Client::action_submit(const seda::IEvent::Ptr &e)
{
  DMLOG(DEBUG,"sending submit message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_cancel(const seda::IEvent::Ptr &e)
{
  DMLOG(DEBUG,"sending cancel message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_query(const seda::IEvent::Ptr &e)
{
  DMLOG(DEBUG,"sending query message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_retrieve(const seda::IEvent::Ptr &e)
{
  DMLOG(DEBUG,"sending retrieve message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_delete(const seda::IEvent::Ptr &e)
{
  DMLOG(DEBUG,"sending delete message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_store_reply(const seda::IEvent::Ptr &reply)
{
  boost::unique_lock<boost::mutex> lock(mtx_);

  DMLOG(DEBUG,"storing reply message: " << reply->str());
  reply_ = reply;
  cond_.notify_one();
}
