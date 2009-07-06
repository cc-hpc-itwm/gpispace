#include "Client.hpp"

#include <seda/StageRegistry.hpp>
#include <sdpa/LoggingConfigurator.hpp>
#include <sdpa/client/ConfigEvents.hpp>

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
  boost::unique_lock<boost::mutex> lock(mtx_);
  if (dynamic_cast<ConfigOK*>(event.get())) {
    fsm_.ConfigOk();
  } else if (dynamic_cast<ConfigNOK*>(event.get())) {
    fsm_.ConfigNok();
  }
  blocked_ = false;
  cond_.notify_one();
}

void Client::start(/*config*/)
{
  SDPA_LOG_DEBUG("starting up");

  boost::unique_lock<boost::mutex> lock(mtx_);
  blocked_ = true;

  fsm_.Start();
  while (blocked_)
  {
    // wait until configured
    cond_.wait(lock);
  }

  if (config_ok_) {
    SDPA_LOG_INFO("configuration was ok");
  } else {
    throw std::runtime_error("configuration was not ok");
  }
}

void Client::shutdown()
{
  fsm_.Shutdown();
}

sdpa::job_id_t Client::submitJob(const job_desc_t &desc)
{
  SDPA_LOG_DEBUG("submitting job with description = " << desc);
  // lock()
  fsm_.Submit(desc);
  // wait for ack or error reply
  SDPA_LOG_DEBUG("waiting for a reply");
  // if no reply: throw error
  // else return reply.job-id()
  return sdpa::job_id_t("dummy-job-id");
}

void Client::cancelJob(const job_id_t &jid)
{
  fsm_.Submit(jid);
}

int Client::queryJob(const job_id_t &jid)
{
  fsm_.Query(jid);
  return 0;
}

void Client::deleteJob(const job_id_t &jid)
{
  fsm_.Delete(jid);
}

sdpa::client::Client::result_t Client::retrieveResults(const job_id_t &jid)
{
  fsm_.Retrieve(jid);
  return result_t("");
}

void Client::action_configure()
{
  // configure logging according to config
  sdpa::logging::Configurator::configure(/*use default config for now*/);  
  SDPA_LOG_DEBUG("configuring my environment");
  // send event to myself
  client_stage_->send(seda::IEvent::Ptr(new ConfigOK()));
}

void Client::action_config_ok()
{
  SDPA_LOG_DEBUG("config ok");
  config_ok_ = true;
}

void Client::action_config_nok()
{
  SDPA_LOG_DEBUG("config not ok");
  config_ok_ = false;
}

void Client::action_shutdown()
{
  SDPA_LOG_DEBUG("shutting down");
  client_stage_->stop();
  seda::StageRegistry::instance().remove(client_stage_);
  client_stage_.reset();
}

void Client::action_submit(const job_desc_t &desc)
{
  SDPA_LOG_DEBUG("sending submit message");
  // send submitJob event
}

void Client::action_cancel(const job_id_t &jid)
{
  SDPA_LOG_DEBUG("sending cancel message");
}

void Client::action_query(const job_id_t &jid)
{
  SDPA_LOG_DEBUG("sending query message");
}

void Client::action_retrieve(const job_id_t &jid)
{
  SDPA_LOG_DEBUG("sending retrieve message");
}

void Client::action_delete(const job_id_t &jid)
{
  SDPA_LOG_DEBUG("sending delete message");
}
