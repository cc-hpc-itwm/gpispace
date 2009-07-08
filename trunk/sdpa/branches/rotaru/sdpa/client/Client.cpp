#include "Client.hpp"

#include <seda/StageRegistry.hpp>
#include <sdpa/LoggingConfigurator.hpp>

using namespace sdpa::client;

Client::ptr_t Client::create(const std::string &name_prefix) {
  // warning: we introduce a cycle here, we have to resolve it during shutdown!
  Client::ptr_t client(new Client(name_prefix));
  seda::Stage::Ptr client_stage(new seda::Stage(name_prefix+"", client));
  client->setStage(client_stage);
  seda::StageRegistry::instance().insert(client_stage);
  client_stage->start();
  return client;
}

void Client::perform(const seda::IEvent::Ptr &event)
{

}

void Client::start(/*config*/)
{
  SDPA_LOG_DEBUG("starting up");
  // lock()
  fsm_.Start();
  // wait until configured
  fsm_.ConfigOk();
  // unlock()
  // return
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
  // wait for reply
  SDPA_LOG_DEBUG("waiting for a reply");
  // if no reply: throw error
  // else return reply.job-id()
  return sdpa::job_id_t("dummy-job-id");
}

void Client::cancelJob(const job_id_t &jid)
{
  fsm_.Cancel(jid);
}

int Client::queryJob(const job_id_t &jid)
{
  fsm_.Query(jid);
}

void Client::deleteJob(const job_id_t &jid)
{
  fsm_.Delete(jid);
}

sdpa::client::Client::result_t Client::retrieveResults(const job_id_t &jid)
{
  fsm_.Retrieve(jid);
}

void Client::action_configure()
{
  // configure logging according to config
  sdpa::logging::Configurator::configure(/*use default config for now*/);  
  SDPA_LOG_DEBUG("configuring my environment");
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
}

void Client::action_submit(const job_desc_t &desc)
{
  SDPA_LOG_DEBUG("sending submit message");
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
