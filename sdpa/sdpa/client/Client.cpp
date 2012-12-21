#include "Client.hpp"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/lexical_cast.hpp>

#include <fhglog/fhglog.hpp>
#include <fhglog/Configuration.hpp>

#include <seda/StageRegistry.hpp>
//#include <seda/comm/comm.hpp>
//#include <seda/comm/ConnectionFactory.hpp>
//#include <seda/comm/ConnectionStrategy.hpp>
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

Client::ptr_t Client::create(const std::string &name_prefix
                           , const std::string &output_stage) {
  // warning: we introduce a cycle here, we have to resolve it during shutdown!
  Client::ptr_t client(new Client(name_prefix, output_stage));
  seda::Stage::Ptr client_stage(new seda::Stage(name_prefix, client));
  client->setStage(client_stage);
  seda::StageRegistry::instance().insert(client_stage);
  client_stage->start();
  return client;
}

Client::Client(const std::string &a_name, const std::string &output_stage)
  : seda::Strategy(a_name)
  , name_(a_name)
  , output_stage_(output_stage)
  , fsm_(*this)
  , timeout_(5000U)
  , my_location_("127.0.0.1:0")
{}

Client::~Client()
{
  MLOG(TRACE, "destroying client api");
}

void Client::perform(const seda::IEvent::Ptr &event)
{
  DMLOG(TRACE, "got event: " << event->str());
  if (dynamic_cast<ConfigOK*>(event.get())) {
    DMLOG(TRACE, "ok");
    fsm_.ConfigOk(event);
  } else if (dynamic_cast<ConfigNOK*>(event.get())) {
    DMLOG(TRACE,"nok");
    fsm_.ConfigNok(event);
  } else if (StartUp * startup = dynamic_cast<StartUp*>(event.get())) {
    DMLOG(TRACE,"start");
    fsm_.Start(startup->config());
  } else if (/* Shutdown *shut = */ dynamic_cast<Shutdown*>(event.get())) {
    DMLOG(TRACE,"shut");
    fsm_.Shutdown();
  } else if (dynamic_cast<se::SubmitJobEvent*>(event.get())) {
    DMLOG(TRACE,"sub");
    fsm_.Submit(event);
  } else if (dynamic_cast<se::SubmitJobAckEvent*>(event.get())) {
    DMLOG(TRACE,"ack");
    fsm_.SubmitAck(event);
  } else if (dynamic_cast<se::SubscribeEvent*>(event.get())) {
    DMLOG(TRACE,"subscribe");
    fsm_.Subscribe(event);
  } else if (dynamic_cast<se::SubscribeAckEvent*>(event.get())) {
    DMLOG(TRACE,"subscribe");
    fsm_.SubscribeAck(event);
  } else if (dynamic_cast<se::QueryJobStatusEvent*>(event.get())) {
    DMLOG(TRACE,"qstat");
    fsm_.Query(event);
  } else if (dynamic_cast<se::JobStatusReplyEvent*>(event.get())) {
    DMLOG(TRACE,"rstat");
    fsm_.StatusReply(event);
  } else if (dynamic_cast<se::CancelJobEvent*>(event.get())) {
    DMLOG(TRACE,"kill");
    fsm_.Cancel(event);
  } else if (dynamic_cast<se::CancelJobAckEvent*>(event.get())) {
    DMLOG(TRACE,"ack");
    fsm_.CancelAck(event);
  } else if (dynamic_cast<se::RetrieveJobResultsEvent*>(event.get())) {
    DMLOG(TRACE,"get");
    fsm_.Retrieve(event);
  } else if (dynamic_cast<se::JobResultsReplyEvent*>(event.get())) {
    DMLOG(TRACE,"get");
    fsm_.Results(event);
  } else if (dynamic_cast<se::DeleteJobEvent*>(event.get())) {
    DMLOG(TRACE,"del");
    fsm_.Delete(event);
  } else if (dynamic_cast<se::DeleteJobAckEvent*>(event.get())) {
    DMLOG(TRACE,"ack");
    fsm_.DeleteAck(event);
  } else if (dynamic_cast<se::ErrorEvent*>(event.get())) {
    DMLOG(TRACE,"err");
    fsm_.Error(event);
  }
  else
  {
    DMLOG(TRACE,"unkown");
    fsm_.Unknown(event);
  }
}

void Client::start(const config_t & config) throw (ClientException)
{
  DMLOG(DEBUG, "starting up");

  client_stage_->send(seda::IEvent::Ptr(new StartUp(config)));

  DMLOG(TRACE, "waiting until configuration is done.");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply(0));
    // check event type
    if (dynamic_cast<ConfigOK*>(reply.get()))
    {
      MLOG(INFO,"configuration was ok");
    }
    else if (ConfigNOK *cfg_nok = dynamic_cast<ConfigNOK*>(reply.get()))
    {
      MLOG(ERROR, "configuration had errors: " << cfg_nok->reason());
      throw ConfigError(cfg_nok->reason());
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
  client_stage_->stop();
  seda::StageRegistry::instance().remove(client_stage_);
  client_stage_.reset();

  /* somehow times out from time to time...
  client_stage_->send(seda::IEvent::Ptr(new Shutdown()));
  try
  {
    wait_for_reply();
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("shutdown", "this call should never timeout!");
  }
  */
}

void Client::subscribe(const job_id_list_t& listJobIds) throw (ClientException)
{
        clear_reply();
  MLOG(DEBUG,"attempting to subscribe ... " );
  client_stage_->send(seda::IEvent::Ptr(new se::SubscribeEvent(name(), orchestrator_, listJobIds)));
  DMLOG(TRACE,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (se::SubscribeAckEvent *sj_ack = dynamic_cast<se::SubscribeAckEvent*>(reply.get()))
    {
      MLOG(TRACE,"got SubscribeAckEvent: "<< sj_ack->from()<< " -> "<< sj_ack->to());
    }
    else if (se::ErrorEvent *err = dynamic_cast<se::ErrorEvent*>(reply.get()))
    {
      throw ClientException( "error during subscribe: reason := "+ err->reason()
                           + " code := "+ boost::lexical_cast<std::string>(err->error_code()));
    }
    else
    {
      MLOG(ERROR, "unexpected reply: " << reply->str());
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
        boost::unique_lock<boost::mutex> lock(mtx_);

        while (reply_.get() == NULL)
        {
                if (t)
                {
                        const boost::system_time to(boost::get_system_time() + boost::posix_time::milliseconds(t));
                        if (!cond_.timed_wait(lock, to))
                        {
                                if (reply_.get() != NULL)
                                        break;
                                else
                                        throw Timedout("did not receive reply");
                        }
                }
                else
                        cond_.wait(lock);
        }

        seda::IEvent::Ptr ret(reply_);
        reply_.reset();
        return ret;
}

void Client::clear_reply()
{
        boost::unique_lock<boost::mutex> lock(mtx_);
        if (reply_)
        {
                LOG(WARN, "clearing old reply: " << reply_->str());
        }
        reply_.reset();
}

sdpa::job_id_t Client::submitJob(const job_desc_t &desc) throw (ClientException)
{
        clear_reply();
  //MLOG(DEBUG,"submitting job with description = " << desc);
  MLOG(DEBUG,"submitting new job ... " );
  client_stage_->send(seda::IEvent::Ptr(new se::SubmitJobEvent(name(), orchestrator_, "", desc, "")));
  DMLOG(TRACE,"waiting for a reply");
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
      MLOG(ERROR, "unexpected reply: " << reply->str());
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
  MLOG(DEBUG,"cancelling job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::CancelJobEvent( name()
                                                              , orchestrator_
                                                              , jid
                                                              , "user cancel"
                                                              )
                                       )
                     );
  DMLOG(TRACE,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (/* se::CancelJobAckEvent *ack = */ dynamic_cast<se::CancelJobAckEvent*>(reply.get()))
    {
      DMLOG(DEBUG,"cancellation has been acknowledged");
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
      MLOG(ERROR, "unexpected reply: " << reply->str());
      throw ClientException("got an unexpected reply to CancelJob: " + reply->str());
    }
  }
  catch(const Timedout &)
  {
    throw ApiCallFailed("cancelJob");
  }
}

std::string Client::queryJob(const job_id_t &jid) throw (ClientException)
{
        clear_reply();
        MLOG(DEBUG,"querying status of job: " << jid);
        client_stage_->send(seda::IEvent::Ptr(new se::QueryJobStatusEvent(name()
                                                                 , orchestrator_
                                                                 , jid)));
  DMLOG(TRACE,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (se::JobStatusReplyEvent *status = dynamic_cast<se::JobStatusReplyEvent*>(reply.get()))
    {
      DMLOG(DEBUG,"got status for " << status->job_id() << ": " << status->status());
      return status->status();
    }
    else if (se::ErrorEvent *err = dynamic_cast<se::ErrorEvent*>(reply.get()))
    {
      throw ClientException( "error during query: reason := "
                           + err->reason()
                           + " code := "
                           + boost::lexical_cast<std::string>(err->error_code())
                           );
    }
    else
    {
      MLOG(ERROR, "unexpected reply: " << reply->str());
      throw ClientException("got an unexpected reply to QueryJob: " + reply->str());
    }
  }
  catch (const Timedout &)
  {
    throw ApiCallFailed("queryJob");
  }
}

int Client::queryJob(const job_id_t &jid, job_info_t &info)
{
  clear_reply();
  MLOG(DEBUG,"querying status of job: " << jid);
  client_stage_->send
    (seda::IEvent::Ptr(new se::QueryJobStatusEvent(name()
                                                  , orchestrator_
                                                  , jid)
                      )
    );
  DMLOG(TRACE,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (se::JobStatusReplyEvent *status
       = dynamic_cast<se::JobStatusReplyEvent*>(reply.get()))
    {
      DMLOG( DEBUG
           ,"got status for " << status->job_id()
           << ": " << status->status()
           );
      info.error_code = status->error_code();
      info.error_message = status->error_message();

      return sdpa::status::read(status->status());
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
      MLOG(ERROR, "unexpected reply: " << reply->str());
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
  MLOG(DEBUG,"deleting job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::DeleteJobEvent(name(), orchestrator_, jid)));
  DMLOG(TRACE,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
    if (/* se::DeleteJobAckEvent *ack = */ dynamic_cast<se::DeleteJobAckEvent*>(reply.get()))
    {
        MLOG(DEBUG,"deletion of job has been acknowledged");
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
      MLOG(ERROR, "unexpected reply: " << reply->str());
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
  MLOG(DEBUG,"retrieving results of job: " << jid);
  client_stage_->send(seda::IEvent::Ptr(new se::RetrieveJobResultsEvent(name()
                                                                      , orchestrator_
                                                                      , jid)));
  DMLOG(TRACE,"waiting for a reply");
  try
  {
    seda::IEvent::Ptr reply(wait_for_reply());
    // check event type
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
      MLOG(ERROR, "unexpected reply: " << reply->str());
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
  MLOG(INFO, "configuring my environment");

  if (cfg.is_set("network.timeout"))
  {
    timeout_ = cfg.get<unsigned int>("network.timeout");
    MLOG(DEBUG, "set timeout to: " << timeout_);
  }

  if (cfg.is_set("orchestrator"))
  {
    orchestrator_ = cfg.get<std::string>("orchestrator");
    MLOG(DEBUG, "using orchestrator: " << orchestrator_);
  }

  if (cfg.is_set("location"))
  {
    my_location_ = cfg.get<std::string>("location");
    MLOG(DEBUG, "using location: " << my_location_);
  }

  if (cfg.is_set("network.enable"))
  {
    action_configure_network(cfg);
  }

  if (orchestrator_.empty())
  {
    MLOG(ERROR, "no orchestrator specified!");
    client_stage_->send(seda::IEvent::Ptr(new ConfigNOK("no orchestrator specified!")));
  }
  else
  {
    client_stage_->send(seda::IEvent::Ptr(new ConfigOK()));
  }
}

void Client::action_configure_network(const config_t &cfg)
{
  LOG(DEBUG, "configuring network components...");

  sdpa::com::NetworkStrategy::ptr_t net
    (new sdpa::com::NetworkStrategy( client_stage_->name()
                                   , client_stage_->name() //"sdpac" // TODO encode user, pid, etc
                                   , fhg::com::host_t ("127.0.0.1")
                                   , fhg::com::port_t ("0")
                                   )
    );
  seda::Stage::Ptr output (new seda::Stage(output_stage_, net));
  seda::StageRegistry::instance().insert (output);
  output->start ();

  /*
  const std::string net_stage_name(client_stage_->name()+".from-net");
  {
    LOG(TRACE, "setting up decoding...");
    seda::ForwardStrategy::Ptr to_client(new seda::ForwardStrategy(client_stage_->name()));
    sdpa::events::DecodeStrategy::ptr_t decode(new sdpa::events::DecodeStrategy(net_stage_name, to_client));
    seda::Stage::Ptr from_net(new seda::Stage(net_stage_name, decode));
    seda::StageRegistry::instance().insert(from_net);
    from_net->start();
  }

  {
    DMLOG(TRACE, "setting up output stage...");
    seda::comm::ConnectionFactory connFactory;
    seda::comm::ConnectionParameters params("udp", my_location_, client_stage_->name());
    seda::comm::Connection::ptr_t conn = connFactory.createConnection(params);
    seda::comm::ConnectionStrategy::ptr_t conn_s(new seda::comm::ConnectionStrategy(net_stage_name, conn));
    sdpa::events::EncodeStrategy::ptr_t encode(new sdpa::events::EncodeStrategy(net_stage_name, conn_s));

    if (cfg.is_set("network.location"))
    {
      const std::vector<std::string> &locations(cfg.get<std::vector<std::string> >("network.location"));
      for (std::vector<std::string>::const_iterator loc(locations.begin()); loc != locations.end(); ++loc)
      {
        const std::string n(loc->substr(0, loc->find(':')));
        const std::string l(loc->substr(n.size()+1));
        LOG(TRACE, "inserting location information: " << n << " -> " << l);
        conn->locator()->insert(n, l);
      }
    }

    seda::Stage::Ptr output(new seda::Stage(output_stage_, encode));
    seda::StageRegistry::instance().insert(output);
    output->start();
  }
  */
  LOG(DEBUG, "network configuration complete");
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
  MLOG(DEBUG,"shutting down");
  DMLOG(TRACE,"waking up api");
  action_store_reply(seda::IEvent::Ptr(new ShutdownComplete()));
}

void Client::action_shutdown_network()
{
  DMLOG(DEBUG, "shutting network compents down...");
  seda::StageRegistry::instance().lookup(output_stage_)->stop();
  //  seda::StageRegistry::instance().lookup(client_stage_->name() + ".from-net")->stop();

  seda::StageRegistry::instance().remove(output_stage_);
  //  seda::StageRegistry::instance().remove(client_stage_->name() + ".from-net");
}

void Client::action_subscribe(const seda::IEvent::Ptr &e)
{
  DMLOG(TRACE,"sending subscribe message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_submit(const seda::IEvent::Ptr &e)
{
  DMLOG(TRACE,"sending submit message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_cancel(const seda::IEvent::Ptr &e)
{
  DMLOG(TRACE,"sending cancel message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_query(const seda::IEvent::Ptr &e)
{
  DMLOG(TRACE,"sending query message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_retrieve(const seda::IEvent::Ptr &e)
{
  DMLOG(TRACE,"sending retrieve message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_delete(const seda::IEvent::Ptr &e)
{
  DMLOG(TRACE,"sending delete message");
  seda::StageRegistry::instance().lookup(output_stage_)->send(e);
}

void Client::action_store_reply(const seda::IEvent::Ptr &reply)
{
  boost::unique_lock<boost::mutex> lock(mtx_);

  DMLOG(TRACE,"storing reply message: " << reply->str());
  reply_ = reply;
  cond_.notify_one();
}
