#include "Client.hpp"

#include <boost/date_time/posix_time/posix_time_types.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <seda/StageRegistry.hpp>
#include <seda/comm/comm.hpp>
#include <seda/comm/ConnectionFactory.hpp>
#include <seda/comm/ConnectionStrategy.hpp>
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

#include <sdpa/events/CodecStrategy.hpp>

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
  } else if (StartUp * startup = dynamic_cast<StartUp*>(event.get())) {
    DMLOG(DEBUG,"start");
    fsm_.Start(startup->config());
  } else if (/* Shutdown *shut = */ dynamic_cast<Shutdown*>(event.get())) {
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

void Client::start(const config_t & config) throw (ClientException)
{
  DMLOG(DEBUG, "starting up");
//  fhg::log::Configurator::configure(/*use default config for now*/);

  client_stage_->send(seda::IEvent::Ptr(new StartUp(config)));

  DMLOG(DEBUG, "waiting until configuration is done.");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (dynamic_cast<ConfigOK*>(reply.get()))
    {
      MLOG(INFO,"configuration was ok");
    }
    else if (dynamic_cast<ConfigNOK*>(reply.get()))
    {
      throw ConfigError("configuration was not ok");
    }
    else
    {
      throw ClientException("startup failed");
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("start", "this call should never timeout!");
  }
}

void Client::shutdown() throw (ClientException)
{
  client_stage_->send(seda::IEvent::Ptr(new Shutdown()));
  try
  {
    wait_for_reply();
    client_stage_->stop();
    seda::StageRegistry::instance().remove(client_stage_);
    client_stage_.reset();
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("shutdown", "this call should never timeout!");
  }
}

seda::IEvent::Ptr Client::wait_for_reply() throw (Timedout)
{
  boost::unique_lock<boost::mutex> lock(mtx_);
  while (reply_.get() == NULL)
  {
    const boost::system_time to(boost::get_system_time() + boost::posix_time::milliseconds(timeout_));
    if (! cond_.timed_wait(lock, to)) {
      if (reply_.get() != NULL)
      {
        break;
      }
      else
      {
        throw Timedout("did not receive reply");
      }
    }
  }
  seda::IEvent::Ptr ret(reply_);
  reply_.reset();
  return ret;
}

sdpa::job_id_t Client::submitJob(const job_desc_t &desc) throw (ClientException)
{
  MLOG(INFO,"submitting job with description = " << desc);
  client_stage_->send(seda::IEvent::Ptr(new se::SubmitJobEvent(name(), orchestrator_, "", desc)));
  DMLOG(DEBUG,"waiting for a reply");
  // TODO: wait_for_reply(config.get<timeout_t>("sdpa.network.timeout")
  try
  {
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
      MLOG(ERROR, "unexpected reply: " << (reply ? reply->str() : "null"));
      throw ClientException("got an unexpected reply");
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("submitJob");
  }
}

void Client::cancelJob(const job_id_t &jid) throw (ClientException)
{
  MLOG(INFO,"cancelling job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::CancelJobEvent(name()
                                                             , orchestrator_
                                                             , jid)));
  DMLOG(DEBUG,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (/* se::CancelJobAckEvent *ack = */ dynamic_cast<se::CancelJobAckEvent*>(reply.get()))
    {
      DMLOG(DEBUG,"cancellation has been acknowledged");
    }
    else
    {
      MLOG(ERROR, "unexpected reply: " << (reply ? reply->str() : "null"));
      throw ClientException("got an unexpected reply");
    }
  }
  catch(const Timedout &)
  {
    throw ApiCallFailed("cancelJob");
  }
}

std::string Client::queryJob(const job_id_t &jid) throw (ClientException)
{
  MLOG(INFO,"querying status of job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::QueryJobStatusEvent(name()
                                                                 , orchestrator_
                                                                 , jid)));
  DMLOG(DEBUG,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (se::JobStatusReplyEvent *status = dynamic_cast<se::JobStatusReplyEvent*>(reply.get()))
    {
      DMLOG(DEBUG,"got status for " << status->job_id() << ": " << status->status());
      return status->status();
    }
    else
    {
      MLOG(ERROR, "unexpected reply: " << (reply ? reply->str() : "null"));
      throw ClientException("got an unexpected reply");
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("queryJob");
  }
}

void Client::deleteJob(const job_id_t &jid) throw (ClientException)
{
  MLOG(INFO,"deleting job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::DeleteJobEvent(name()
                                                             , orchestrator_
                                                             , jid)));
  DMLOG(DEBUG,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (/* se::DeleteJobAckEvent *ack = */ dynamic_cast<se::DeleteJobAckEvent*>(reply.get()))
    {
      DMLOG(DEBUG,"deletion of job has been acknowledged");
    }
    else
    {
      MLOG(ERROR, "unexpected reply: " << (reply ? reply->str() : "null"));
      throw ClientException("got an unexpected reply");
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("deleteJob");
  }
}

sdpa::client::result_t Client::retrieveResults(const job_id_t &jid) throw (ClientException)
{
  MLOG(INFO,"retrieving results of job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::RetrieveJobResultsEvent(name()
                                                                      , orchestrator_
                                                                      , jid)));
  DMLOG(DEBUG,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (se::JobResultsReplyEvent *res = dynamic_cast<se::JobResultsReplyEvent*>(reply.get()))
    {
      DMLOG(DEBUG,"results: " << res->result());
      return res->result();
    }
    else
    {
      MLOG(ERROR, "unexpected reply: " << (reply ? reply->str() : "null"));
      throw ClientException("got an unexpected reply");
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("retrieveResults");
  }
}

void Client::action_configure(const config_t &cfg)
{
  MLOG(INFO, "configuring my environment");
  
  if (cfg.count("network.timeout"))
  {
    timeout_ = cfg["network.timeout"].as<unsigned int>();
    MLOG(DEBUG, "set timeout to: " << timeout_);
  }

  if (cfg.count("orchestrator"))
  {
    orchestrator_ = cfg["orchestrator"].as<std::string>();
    MLOG(DEBUG, "using orchestrator: " << orchestrator_);
  }

  if (cfg.count("network.enable"))
  {
    action_configure_network(cfg);
  }

  MLOG(DEBUG, "config: timeout = " << timeout_);
  MLOG(DEBUG, "config: orchestrator = " << orchestrator_);
  MLOG(DEBUG, "config: location = " << my_location_);
  
  // send event to myself
  if (orchestrator_.empty())
  {
    client_stage_->send(seda::IEvent::Ptr(new ConfigNOK()));
  }
  else
  {
    client_stage_->send(seda::IEvent::Ptr(new ConfigOK()));
  }
}

void Client::action_configure_network(const config_t &cfg)
{
  MLOG(INFO, "configuring network components...");
  const std::string net_stage_name(client_stage_->name()+".from-net");
  {
    DMLOG(INFO, "setting up decoding...");
    seda::ForwardStrategy::Ptr to_client(new seda::ForwardStrategy(client_stage_->name()));
    sdpa::events::DecodeStrategy::ptr_t decode(new sdpa::events::DecodeStrategy(net_stage_name, to_client));
    seda::Stage::Ptr from_net(new seda::Stage(net_stage_name, decode));
    seda::StageRegistry::instance().insert(from_net);
    from_net->start();
  }

  {
    DMLOG(INFO, "setting up output stage...");
    seda::comm::ConnectionFactory connFactory;
    seda::comm::ConnectionParameters params("udp", my_location_, client_stage_->name());
    seda::comm::Connection::ptr_t conn = connFactory.createConnection(params);
    seda::comm::ConnectionStrategy::ptr_t conn_s(new seda::comm::ConnectionStrategy(net_stage_name, conn));
    sdpa::events::EncodeStrategy::ptr_t encode(new sdpa::events::EncodeStrategy(net_stage_name, conn_s));

    if (cfg.count("network.location"))
    {
      const std::vector<std::string> &locations(cfg["network.location"].as<std::vector<std::string> >());
      for (std::vector<std::string>::const_iterator loc(locations.begin()); loc != locations.end(); ++loc)
      {
        const std::string n(loc->substr(0, loc->find(':')));
        const std::string l(loc->substr(n.size()+1));
        MLOG(DEBUG, "inserting location information: " << n << " -> " << l);
        conn->locator()->insert(n, l);
      }
    }

    seda::Stage::Ptr output(new seda::Stage(output_stage_, encode));
    seda::StageRegistry::instance().insert(output);
    output->start();
  }
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
  DMLOG(DEBUG,"waking up api");
  action_store_reply(seda::IEvent::Ptr(new ShutdownComplete()));
}

void Client::action_shutdown_network()
{
  DMLOG(INFO, "shutting network compents down...");
  seda::StageRegistry::instance().lookup(output_stage_)->stop();
  seda::StageRegistry::instance().lookup(client_stage_->name() + ".from-net")->stop();

  seda::StageRegistry::instance().remove(output_stage_);
  seda::StageRegistry::instance().remove(client_stage_->name() + ".from-net");
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
