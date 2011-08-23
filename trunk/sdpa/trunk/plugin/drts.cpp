#include "drts.hpp"
#include "master.hpp"

#include <errno.h>

#include <map>
#include <list>
#include <vector>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/util/thread/queue.hpp>

//#include <gpi-space/pc/plugin/gpi.hpp>
#include <sdpa/uuidgen.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/events.hpp>

#include <fhgcom/peer.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>

class DRTSImpl : FHG_PLUGIN
               , public drts::DRTS
               , public sdpa::events::EventHandler
{
  typedef fhg::thread::queue< sdpa::events::SDPAEvent::Ptr
                            , std::list
                            > event_queue_t;

public:
  FHG_PLUGIN_START()
  {
    m_shutting_down = false;
    m_my_name = fhg_kernel()->get("name", "drts");

    assert (! m_event_thread);

    m_event_thread.reset(new boost::thread(&DRTSImpl::event_thread, this));

    // initialize peer
    m_peer.reset (new fhg::com::peer_t ( m_my_name
                                       , fhg::com::host_t(fhg_kernel()->get("host", "*"))
                                       , fhg::com::port_t(fhg_kernel()->get("port", "0"))
                                       )
                 );
    m_peer_thread.reset(new boost::thread(&fhg::com::peer_t::run, m_peer));
    try
    {
      m_peer->start();
    }
    catch (std::exception const &ex)
    {
      LOG(ERROR, "could not start peer: " << ex.what());
      FHG_PLUGIN_FAILED(EAGAIN);
    }

    start_receiver();

    {
      const std::string master_name (fhg_kernel()->get("master", "aggregator"));
      LOG(INFO, "adding master \"" << master_name << "\"");
      m_masters.insert
        (std::make_pair(master_name, boost::shared_ptr<drts::Master>(new drts::Master(master_name))));
    }

    start_connect ();

    // initialize statemachine
    //   not-connected
    //     schedule registration
    //   connected
    //     poll for jobs -> modify submitjob so that more than one job can be sent

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    // cancel running jobs etc.
    m_shutting_down = true;

    if (m_event_thread)
    {
      m_event_thread->interrupt();
      m_event_thread->join();
      m_event_thread.reset();
    }

    if (m_peer)
    {
      m_peer->stop();
    }

    if (m_peer_thread)
    {
      m_peer_thread->interrupt();
      m_peer_thread->join();
    }

    m_peer_thread.reset();
    m_peer.reset();

    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if ("gpi" == plugin)
    {
      LOG(INFO, "gained capability: gpi");
    }
  }

  FHG_ON_PLUGIN_UNLOAD(plugin)
  {
  }

  FHG_ON_PLUGIN_PREUNLOAD(plugin)
  {
    if ("gpi" == plugin)
    {
      LOG(INFO, "lost capability: gpi");

    }
  }

  int exec (drts::job_desc_t const &, drts::job_id_t &, drts::JobListener *)
  {
    // parse job
    //   if ok:
    //      assign job id
    //      state == pending
    //      schedule job with attached listener
    //   else:
    //      return -EINVAL
    return -EPERM;
  }

  drts::status::status_t query (drts::job_id_t const & jobid)
  {
    return drts::status::FAILED;
  }

  int cancel (drts::job_id_t const & jobid)
  {
    return -ESRCH;
  }

  int results (drts::job_id_t const & jobid, std::string &)
  {
    return -ESRCH;
  }

  int remove (drts::job_id_t const & jobid)
  {
    return -ESRCH;
  }

  int add_agent (std::string const & name)
  {
    // 1. remember agent in agent map
    //   state - not connected
    // 2. send registration event to agent
    //   re-schedule registration send
    // 3. when we receive an acknowledge
    //   change agents state to 'connected'
    //   no re-schedule of registration
    // 4. on connection lost -> del_agent
    return 0;
  }

  int del_agent (std::string const & name)
  {
    //  change state to 'not-connected'
    //  if there are jobs from that agent:
    //     remember finished jobs (with timestamp)
    //     what should happen with not yet executed jobs?
    //  else: remove agent
    return 0;
  }

  // event handler callbacks
  virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent *)
  {
  }

  virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent *)
  {
  }

  virtual void handleConfigNokEvent(const sdpa::events::ConfigNokEvent *)
  {
  }

  virtual void handleConfigOkEvent(const sdpa::events::ConfigOkEvent *)
  {

  }
  virtual void handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent *)
  {

  }

  virtual void handleConfigRequestEvent(const sdpa::events::ConfigRequestEvent *)
  {
  }

  virtual void handleDeleteJobAckEvent(const sdpa::events::DeleteJobAckEvent *)
  {
  }

  virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent *) {}
  virtual void handleErrorEvent(const sdpa::events::ErrorEvent *)
  {

  }
  virtual void handleInterruptEvent(const sdpa::events::InterruptEvent *){}
  virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent *){}
  virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent *) {}
  virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent *) {}
  virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent *) {}
  virtual void handleJobResultsReplyEvent(const sdpa::events::JobResultsReplyEvent *) {}
  virtual void handleJobStatusReplyEvent(const sdpa::events::JobStatusReplyEvent *) {}
  virtual void handleLifeSignEvent(const sdpa::events::LifeSignEvent *) {}
  virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent *) {}
  virtual void handleRequestJobEvent(const sdpa::events::RequestJobEvent *) {}
  virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent *) {}
  virtual void handleRunJobEvent(const sdpa::events::RunJobEvent *) {}
  virtual void handleStartUpEvent(const sdpa::events::StartUpEvent *) {}
  virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent *) {}
  virtual void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent *) {}
  virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent *e)
  {
    LOG(INFO, "successfully connected to " << e->from());
    m_connected = true;

    // start to poll
  }
  virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent *e)
  {
    LOG(WARN, "worker tried to register: " << e->from());
    /*
    send_event (new sdpa::events::ErrorEvent( m_my_name
                                            , e->from()
                                            , sdpa::events::ErrorEvent::SDPA_EPERM
                                            , "you are not allowed to connect"
                                            )
               );
    */
  }
  virtual void handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent*) {}
  virtual void handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent*) {}
private:
  void event_thread ()
  {
    for (;;)
    {
      try
      {
        sdpa::events::SDPAEvent::Ptr evt(m_event_queue.get());
        evt->handleBy(this);
      }
      catch (boost::thread_interrupted const & irq)
      {
        LOG(TRACE, "event handler interrupted...");
        throw;
      }
      catch (std::exception const & ex)
      {
        LOG(WARN, "event could not be handled: " << ex.what());
      }
    }
  }

  void do_start()
  {
    // start scheduler threads (configured #)
    // initialize gui-observer if requested
  }

  void start_connect ()
  {
    // bool retry=false
    // for each configured parent
    //    if !connected
    //        send registration
    //        retry = true
    // if retry: reschedule start-connect

    if (! m_connected)
    {
      const std::string parent("aggregator");

      sdpa::events::WorkerRegistrationEvent::Ptr evt
        (new sdpa::events::WorkerRegistrationEvent( m_my_name
                                                  , parent
                                                  )
        );
      evt->rank() = 0;
      evt->capacity() = 2;
      sdpa::uuidgen gen;
      evt->capabilities().insert(sdpa::capability_t(gen().str(), "drts"));
      evt->capabilities().insert(sdpa::capability_t(gen().str(), "gpi"));

      LOG(INFO, "start_connect time = " << time(NULL));

      send_event(evt);

      fhg_kernel()->schedule (boost::bind( &DRTSImpl::start_connect
                                         , this
                                         )
                             , 5
                             );
    }
  }

  void start_receiver()
  {
    m_peer->async_recv(&m_message, boost::bind( &DRTSImpl::handle_recv
                                              , this
                                              , _1
                                              )
                      );
  }

  void handle_recv (boost::system::error_code const & ec)
  {
    static sdpa::events::Codec codec;

    if (! ec)
    {
      // convert m_message to event
      try
      {
        dispatch_event
          (sdpa::events::SDPAEvent::Ptr
          (codec.decode (std::string ( m_message.data.begin()
                                     , m_message.data.end()
                                     )
                        )
          ));
      }
      catch (std::exception const & ex)
      {
        LOG(WARN, "could not handle incoming message: " << ex.what());
      }
      start_receiver();
    }
    else if (! m_shutting_down)
    {
      const fhg::com::p2p::address_t & addr = m_message.header.src;
      if (addr != m_peer->address())
      {
        LOG(WARN, "connection to " << m_peer->resolve (addr, "*unknown*") << " lost: " << ec);

        m_connected = false;
        fhg_kernel()->schedule (boost::bind( &DRTSImpl::start_connect
                                           , this
                                           )
                               , 5
                               );
        start_receiver();
      }
      else
      {
        LOG(TRACE, m_peer->name() << " is shutting down");
      }
    }
  }

  int send_event (sdpa::events::SDPAEvent *e)
  {
    return send_event(sdpa::events::SDPAEvent::Ptr(e));
  }

  int send_event (sdpa::events::SDPAEvent::Ptr const & evt)
  {
    static sdpa::events::Codec codec;

    const std::string encoded_evt (codec.encode(evt.get()));

    try
    {
      m_peer->send (evt->to(), encoded_evt);
    }
    catch (std::exception const &ex)
    {
      LOG(WARN, "could not send " << evt->str() << " to " << evt->to() << ": " << ex.what());
      return -ESRCH;
    }

    return 0;
  }

  void dispatch_event (sdpa::events::SDPAEvent::Ptr const &evt)
  {
    LOG(TRACE, "received event: " << evt->str());
    m_event_queue.put(evt);
  }

  bool m_shutting_down;
  bool m_connected;
  boost::shared_ptr<boost::thread>    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  std::string m_my_name;
  std::map<std::string, boost::shared_ptr<drts::Master> > m_masters;

  event_queue_t m_event_queue;
  boost::shared_ptr<boost::thread>    m_event_thread;

  // workflow engine
  // module loader
  // connections to other agents
  // jobs + their states
  // job-queue(s) (1 per core?)
};

EXPORT_FHG_PLUGIN( drts
                 , DRTSImpl
                 , "provides access to the distributed runtime-system"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "v.0.0.1"
                 , "NA"
                 , "kvs"
                 , ""
                 );
