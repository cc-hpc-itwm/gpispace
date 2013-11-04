#include "drts.hpp"
#include "master.hpp"
#include "job.hpp"
#include "wfe.hpp"
#include "observable.hpp"
#include "drts_callbacks.h"

#include <errno.h>

#include <map>
#include <list>
#include <vector>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/capability.hpp>
#include <fhg/util/thread/queue.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/util/read_bool.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/threadname.hpp>
#include <fhg/error_codes.hpp>

#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/events.hpp>

#include <fhgcom/peer.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <sdpa/types.hpp>

#include <gspc/net.hpp>

class DRTSImpl : FHG_PLUGIN
               , public drts::DRTS
               , public sdpa::events::EventHandler
               , public observe::Observable
{
  typedef boost::mutex mutex_type;
  typedef boost::condition_variable condition_type;
  typedef boost::unique_lock<mutex_type> lock_type;

  typedef fhg::thread::queue< sdpa::events::SDPAEvent::Ptr
                            , std::list
                            > event_queue_t;

  typedef boost::shared_ptr<drts::Master> master_ptr;
  typedef std::map< std::string
                  , master_ptr
                  > map_of_masters_t;

  typedef boost::shared_ptr<drts::Job> job_ptr_t;
  typedef std::map< std::string
                  , job_ptr_t
                  > map_of_jobs_t;
  typedef fhg::thread::queue< job_ptr_t
                            , std::list
                            > job_queue_t;

  typedef std::pair<sdpa::Capability, fhg::plugin::Capability*> capability_info_t;
  typedef std::map<std::string, capability_info_t> map_of_capabilities_t;
public:
  FHG_PLUGIN_START()
  {
    m_shutting_down = false;
    m_job_in_progress = false;
    m_graceful_shutdown_requested = false;

    m_reconnect_counter = 0;
    m_my_name =      fhg_kernel()->get_name ();
    try
    {
      m_backlog_size = fhg_kernel()->get<size_t>("backlog", "3");
    }
    catch (std::exception const &ex)
    {
      MLOG( ERROR
          , "could not parse backlog size: "
          << fhg_kernel()->get("backlog", "3")
          << ": " << ex.what()
          );
      FHG_PLUGIN_FAILED(EINVAL);
    }
    try
    {
      m_terminate_on_failure = fhg::util::read_bool
        (fhg_kernel()->get("terminate_on_failure", "false"));
    }
    catch (std::exception const &ex)
    {
      MLOG( ERROR
          , "could not parse bool from `terminate_on_failure' config key: "
          << ex.what()
          );
      FHG_PLUGIN_FAILED(EINVAL);
    }

    try
    {
      m_max_reconnect_attempts =
        fhg_kernel()->get<std::size_t>("max_reconnect_attempts", "0");
    }
    catch (std::exception const &ex)
    {
      MLOG( ERROR
          , "could not parse 'max_reconnect_attempts' from config: "
          << ex.what()
          );
      FHG_PLUGIN_FAILED(EINVAL);
    }

    m_wfe = fhg_kernel()->acquire<wfe::WFE>("wfe");
    if (0 == m_wfe)
    {
      MLOG(ERROR, "could not access workflow-engine plugin!");
      FHG_PLUGIN_FAILED(ELIBACC);
    }

    // parse virtual capabilities
    {
      std::string virtual_capabilities (fhg_kernel()->get("capabilities", ""));
      std::list<std::string> capability_list;
      fhg::util::split( virtual_capabilities
                      , ","
                      , std::back_inserter(capability_list)
                      );
      BOOST_FOREACH (std::string const & cap, capability_list)
      {
        if (m_virtual_capabilities.find(cap) == m_virtual_capabilities.end())
        {
          std::pair<std::string, std::string> capability_and_type
            = fhg::util::split_string (cap, "-");
          if (capability_and_type.second.empty ())
            capability_and_type.second = "virtual";

          const std::string & cap_name = capability_and_type.first;
          const std::string & cap_type = capability_and_type.second;

          DMLOG ( TRACE
                , "adding capability: " << cap_name
                << " of type: " << cap_type
                );

          m_virtual_capabilities.insert
            (std::make_pair ( cap
                            , std::make_pair ( sdpa::Capability ( cap_name
                                                                , cap_type
                                                                , m_my_name
                                                                )
                                             , new fhg::plugin::Capability( cap_name
                                                                          , cap_type
                                                                          )
                                             )
                            )
            );
        }
      }
    }

    // TODO: add
    //
    //      acquire_all<T>() -> [(name, T*)]
    //
    // to get access to all plugins of a particular type
    fhg::plugin::Capability *cap
      = fhg_kernel()->acquire< fhg::plugin::Capability>("gpi");
    if (cap)
    {
      MLOG( INFO, "gained capability: " << cap->capability_name()
          << " of type " << cap->capability_type()
          );

      lock_type cap_lock(m_capabilities_mutex);
      m_capabilities.insert
        (std::make_pair ( cap->capability_name()
                        , std::make_pair (sdpa::Capability ( cap->capability_name ()
                                                           , cap->capability_type ()
                                                           )
                                         , cap
                                         )
                        )
        );
    }

    assert (! m_event_thread);

    m_event_thread.reset(new boost::thread(&DRTSImpl::event_thread, this));
    fhg::util::set_threadname (*m_event_thread, "[drts-events]");

    // initialize peer
    m_peer.reset
      (new fhg::com::peer_t ( m_my_name
                            , fhg::com::host_t(fhg_kernel()->get("host", "*"))
                            , fhg::com::port_t(fhg_kernel()->get("port", "0"))
                            )
      );
    m_peer_thread.reset(new boost::thread(&fhg::com::peer_t::run, m_peer));
    fhg::util::set_threadname (*m_peer_thread, "[drts-peer]");
    try
    {
      m_peer->start();
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not start peer: " << ex.what());
      FHG_PLUGIN_FAILED(EAGAIN);
    }

    start_receiver();

    bool have_master_with_polling (false);

    {
      const std::string master_names (fhg_kernel()->get("master", ""));

      std::list<std::string> master_list;
      fhg::util::split(master_names, ",", std::back_inserter(master_list));

      BOOST_FOREACH (std::string const & master, master_list)
      {
        try
        {
          master_ptr m (create_master(master));

          if (m_masters.find(m->name()) == m_masters.end())
          {
            DMLOG(TRACE, "adding master \"" << m->name() << "\"");
            m_masters.insert (std::make_pair(m->name(), m));

            have_master_with_polling |= m->is_polling();
          }
          else
          {
            MLOG( WARN
                , "master already specified, ignoring new one: " << master
                );
          }
        }
        catch (std::exception const & ex)
        {
          MLOG(WARN, "could not add master: " << ex.what());
        }
      }

      if (m_masters.empty())
      {
        MLOG(ERROR, "no masters specified, giving up");
        FHG_PLUGIN_FAILED(EINVAL);
      }
    }

    restore_jobs ();

    if (have_master_with_polling)
    {
      m_request_thread.reset
        (new boost::thread(&DRTSImpl::job_requestor_thread, this));
      fhg::util::set_threadname (*m_request_thread, "[drts-requests]");
    }

    m_execution_thread.reset
      (new boost::thread(&DRTSImpl::job_execution_thread, this));
    fhg::util::set_threadname (*m_execution_thread, "[drts-execute]");

    start_connect ();

    gspc::net::handle
      ("/service/drts/capability/add"
      , boost::bind (&DRTSImpl::service_capability_add, this, _1, _2, _3)
      );

    gspc::net::handle
      ("/service/drts/capability/del"
      , boost::bind (&DRTSImpl::service_capability_del, this, _1, _2, _3)
      );

    gspc::net::handle
      ("/service/drts/capability/get"
      , boost::bind (&DRTSImpl::service_capability_get, this, _1, _2, _3)
      );

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    gspc::net::unhandle ("/service/drts/capability/add");
    gspc::net::unhandle ("/service/drts/capability/del");
    gspc::net::unhandle ("/service/drts/capability/get");

    m_shutting_down = true;

    if (m_request_thread)
    {
      m_request_thread->interrupt();
      if (m_request_thread->joinable ())
        m_request_thread->join();
      m_request_thread.reset();
    }

    if (m_execution_thread)
    {
      m_execution_thread->interrupt();
      if (m_execution_thread->joinable ())
        m_execution_thread->join ();
      m_execution_thread.reset();
    }

    if (m_event_thread)
    {
      m_event_thread->interrupt();
      if (m_event_thread->joinable ())
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
      if (m_peer_thread->joinable ())
        m_peer_thread->join();
    }

    m_peer_thread.reset();
    m_peer.reset();

    m_wfe = 0;

    {
      lock_type job_map_lock (m_job_map_mutex);
      fhg_kernel()->storage()->save("jobs", m_jobs);
    }

    {
      while (not m_virtual_capabilities.empty())
      {
        delete m_virtual_capabilities.begin()->second.second;
        m_virtual_capabilities.erase(m_virtual_capabilities.begin());
      }
    }

    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    fhg::plugin::Capability *cap
      = fhg_kernel()->acquire<fhg::plugin::Capability>(plugin);
    if (cap)
    {
      MLOG( INFO
          , "gained capability: " << cap->capability_name()
          << " of type " << cap->capability_type()
          );

      lock_type cap_lock(m_capabilities_mutex);
      m_capabilities.insert
        (std::make_pair ( cap->capability_name()
                        , std::make_pair (sdpa::Capability ( cap->capability_name ()
                                                           , cap->capability_type ()
                                                           )
                                         , cap
                                         )
                        )
        );

      for ( map_of_masters_t::const_iterator master_it(m_masters.begin())
          ; master_it != m_masters.end()
          ; ++master_it
          )
      {
        if (master_it->second->is_connected())
        {
          send_event
            (new sdpa::events::CapabilitiesGainedEvent( m_my_name
                                                      , master_it->first
                                                      , m_capabilities[cap->capability_name ()].first
                                                      )
            );
        }
      }
    }
  }

  FHG_ON_PLUGIN_UNLOAD(plugin)
  {
  }

  FHG_ON_PLUGIN_PREUNLOAD(plugin)
  {
    lock_type cap_lock(m_capabilities_mutex);
    map_of_capabilities_t::iterator cap(m_capabilities.find(plugin));
    if (cap != m_capabilities.end())
    {
      MLOG(INFO, "lost capability: " << plugin);
      MLOG(WARN, "TODO: make sure none of jobs make use of this capability");

      for ( map_of_masters_t::const_iterator master_it(m_masters.begin())
          ; master_it != m_masters.end()
          ; ++master_it
          )
      {
        if (not master_it->second->is_connected())
          continue;

        send_event
          (new sdpa::events::CapabilitiesLostEvent( m_my_name
                                                  , master_it->first
                                                  , cap->second.first
                                                  )
          );
      }

      m_capabilities.erase(cap);
    }
  }

  FHG_ON_SIGNAL(sig, info, ctxt)
  {
    if (sig != SIGUSR2)
      return;

    DMLOG (TRACE, "initiating graceful shutdown due to signal := " << sig);
    bool something_running;
    {
      lock_type lck (m_job_in_progress_mutex);
      something_running = m_job_in_progress;
      m_graceful_shutdown_requested = true;
    }

    if (not something_running)
    {
      fhg_kernel ()->shutdown ();
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
  //    implemented events
  virtual void handleWorkerRegistrationAckEvent
  (const sdpa::events::WorkerRegistrationAckEvent *e)
  {
    map_of_masters_t::iterator master_it (m_masters.find(e->from()));
    if (master_it != m_masters.end())
    {
      if (!master_it->second->is_connected())
      {
        DMLOG(TRACE, "successfully connected to " << master_it->second->name());
        master_it->second->is_connected(true);
        master_it->second->reset_poll_rate();

        notify_capabilities_to_master (master_it->second);
        resend_outstanding_events (master_it->second);

        m_connected_event.notify(master_it->second->name());

        // simulate a new job to wake up requestor thread if necessary
        {
          lock_type lock(m_job_arrived_mutex);
          m_job_arrived.notify_all();
        }
      }

      {
        lock_type lock_reconnect_counter (m_reconnect_counter_mutex);
        m_reconnect_counter = 0;
      }
    }
  }
  virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent *e)
  {
    MLOG(WARN, "worker tried to register: " << e->from());

    send_event
      (new sdpa::events::ErrorEvent( m_my_name
                                   , e->from()
                                   , sdpa::events::ErrorEvent::SDPA_EPERM
                                   , "you are not allowed to connect"
                                   )
      );
  }

  virtual void handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent*)
  {
  }

  virtual void handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent*)
  {
  }

  virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent *)
  {
  }

  virtual void handleErrorEvent(const sdpa::events::ErrorEvent *)
  {
  }

  virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent *)
  {
  }

  virtual void handleRequestJobEvent(const sdpa::events::RequestJobEvent *e)
  {
    send_event
      (new sdpa::events::ErrorEvent( m_my_name
                                   , e->from()
                                   , sdpa::events::ErrorEvent::SDPA_EPERM
                                   , "you are not allowed to request jobs"
                                   )
      );
  }

  virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent *)
  {
  }

  virtual void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent *e)
  {
    // check master
    map_of_masters_t::const_iterator master (m_masters.find(e->from()));

    if (master == m_masters.end())
    {
      MLOG(ERROR, "got SubmitJob from unknown source: " << e->from());
      return;
    }
    else if (! master->second->is_connected())
    {
      MLOG(WARN, "got SubmitJob from not yet connected master: " << e->from());
      send_event
        (new sdpa::events::ErrorEvent( m_my_name
                                     , e->from()
                                     , sdpa::events::ErrorEvent::SDPA_EPERM
                                     , "you are not yet connected"
                                     , e->job_id()
                                     )
        );
      return;
    }
    else if (m_graceful_shutdown_requested)
    {
      MLOG (WARN, "refusing job " << e->job_id () << " : shutting down");
      send_event
        (new sdpa::events::ErrorEvent( m_my_name
                                     , e->from()
                                     , sdpa::events::ErrorEvent::SDPA_EJOBREJECTED
                                     , "I am going to shut down!"
                                     , e->job_id()
                                     )
        );
      return;
    }

    job_ptr_t job (new drts::Job( drts::Job::ID(e->job_id())
                                , drts::Job::Description(e->description())
                                , drts::Job::Owner(e->from())
                                )
                  );

    job->worker_list (e->worker_list ());

    {
      lock_type job_map_lock(m_job_map_mutex);

      if (m_backlog_size && m_pending_jobs.size() >= m_backlog_size)
      {
        MLOG( WARN
            , "cannot accept new job (" << job->id() << "), backlog is full."
            );
        send_event (new sdpa::events::ErrorEvent
                   ( m_my_name
                   , e->from()
                   , sdpa::events::ErrorEvent::SDPA_EJOBREJECTED
                   , "I am busy right now, please try again later!"
                   , e->job_id()
                   ));
        return;
      }
      else
      {
    	sdpa::worker_id_list_t workerList(e->worker_list());
    	DLOG( INFO, "Worker "<<m_my_name<<": received from "
    			             <<e->from()<<" the job " << job->id()
    	   			  	     <<", assigned to "<<workerList );

        send_event (new sdpa::events::SubmitJobAckEvent( m_my_name
                                                       , job->owner()
                                                       , job->id()
                                                       , "empty-message-id"
                                                       )
                   );
        master->second->reset_poll_rate();
        m_jobs.insert (std::make_pair(job->id(), job));

        job->entered(boost::posix_time::microsec_clock::universal_time());

        fhg_kernel()->storage()->save("jobs", m_jobs);

        m_pending_jobs.put(job);
      }
    }

    //    lock_type lock(m_job_arrived_mutex);
    m_job_arrived.notify_all();
  }

  virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent *e)
  {
    // locate the job
    lock_type job_map_lock (m_job_map_mutex);
    map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));

    MLOG(TRACE, "got cancelation request for job: " << e->job_id());

    if (job_it == m_jobs.end())
    {
      DMLOG (WARN, "could not cancel job: " << e->job_id() << ": not found");
      send_event(new sdpa::events::ErrorEvent
                ( m_my_name
                , e->from()
                , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                , "could not find job " + std::string(e->job_id())
                ));
    }
    else if (job_it->second->owner() != e->from())
    {
      DMLOG (ERROR, "could not cancel job: " << e->job_id() << ": not owner");
      send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
      return;
    }
    else
    {
      if (  drts::Job::PENDING
         == job_it->second->cmp_and_swp_state( drts::Job::PENDING
                                             , drts::Job::CANCELED
                                             )
         )
      {
        MLOG(TRACE, "cancelling pending job: " << e->job_id());
        send_event
          (new sdpa::events::CancelJobAckEvent ( m_my_name
                                               , job_it->second->owner()
                                               , job_it->second->id()
                                               , job_it->second->result()
                                               )
          );
      }
      else if (job_it->second->state() == drts::Job::RUNNING)
      {
        MLOG (TRACE, "trying to cancel running job " << e->job_id());
        m_wfe->cancel (e->job_id());
        drts_on_cancel ();
      }
      else if (job_it->second->state() == drts::Job::FAILED)
      {
        MLOG(TRACE, "cancelling already failed job: " << e->job_id());
      }
      else if (job_it->second->state() == drts::Job::CANCELED)
      {
        MLOG(TRACE, "cancelling already cancelled job: " << e->job_id());
      }
      else
      {
        MLOG( WARN
            , "what shall I do with an already computed job? "
            << "(" << e->job_id() << ")"
            );
      }
    }
  }

  virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent *e)
  {
    // locate the job
    lock_type job_map_lock (m_job_map_mutex);
    map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));
    if (job_it == m_jobs.end())
    {
      MLOG( ERROR
          , "could not acknowledge failed job: " << e->job_id() << ": not found"
          );
      send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                 , "could not find job " + std::string(e->job_id())
                 ));
      return;
    }
    else if (job_it->second->owner() != e->from())
    {
      MLOG( ERROR
          , "could not acknowledge failed job: " << e->job_id() << ": not owner"
          );
      send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
      return;
    }

    DMLOG(TRACE, "removing job " << e->job_id());
    m_jobs.erase (job_it);

    fhg_kernel()->storage()->save("jobs", m_jobs);
  }

  virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent *e)
  {
    // locate the job
    lock_type job_map_lock (m_job_map_mutex);
    map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));
    if (job_it == m_jobs.end())
    {
      MLOG( ERROR
          , "could not acknowledge finished job: " << e->job_id()
          << ": not found"
          );
      send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                 , "could not find job " + std::string(e->job_id())
                 ));
      return;
    }
    else if (job_it->second->owner() != e->from())
    {
      MLOG( ERROR
          , "could not acknowledge finished job: " << e->job_id()
          << ": not owner"
          );
      send_event (new sdpa::events::ErrorEvent
                 ( m_my_name
                 , e->from()
                 , sdpa::events::ErrorEvent::SDPA_EPERM
                 , "you are not the owner of job " + std::string(e->job_id())
                 ));
      return;
    }

    DMLOG(TRACE, "removing job " << e->job_id());
    m_jobs.erase (job_it);

    fhg_kernel()->storage()->save("jobs", m_jobs);
  }

  // not implemented events
  virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent *){}
  virtual void handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent *) {}
  virtual void handleConfigRequestEvent(const sdpa::events::ConfigRequestEvent *) {}
  virtual void handleDeleteJobAckEvent(const sdpa::events::DeleteJobAckEvent *) {}
  virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent *) {}
  virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent *) {}
  virtual void handleJobResultsReplyEvent(const sdpa::events::JobResultsReplyEvent *) {}
  virtual void handleJobStatusReplyEvent(const sdpa::events::JobStatusReplyEvent *) {}
  virtual void handleLifeSignEvent(const sdpa::events::LifeSignEvent *) {}
  virtual void handleRunJobEvent(const sdpa::events::RunJobEvent *) {}
  virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent *) {}
private:
  // threads
  void event_thread ()
  {
    for (;;)
    {
      try
      {
        sdpa::events::SDPAEvent::Ptr evt(m_event_queue.get());
        map_of_masters_t::iterator m(m_masters.find(evt->from()));
        if (m != m_masters.end())
        {
          m->second->update_recv();
        }

        evt->handleBy(this);
      }
      catch (boost::thread_interrupted const & irq)
      {
        DMLOG(TRACE, "event handler interrupted...");
        throw;
      }
      catch (std::exception const & ex)
      {
        MLOG(WARN, "event could not be handled: " << ex.what());
      }
    }
  }

  void job_requestor_thread ()
  {
    for (;;)
    {
      {
        lock_type lock(m_job_computed_mutex);
        while (m_backlog_size && (m_pending_jobs.size() >= m_backlog_size))
        {
          MLOG( TRACE
              , "job requestor waits until job queue frees up some slots"
              );
          m_job_computed.wait(lock);
        }
      }

      bool at_least_one_connected = false;
      boost::posix_time::time_duration min_sleep_time
        (boost::posix_time::minutes(5));

      const boost::posix_time::ptime now
        (boost::posix_time::microsec_clock::universal_time());

      for ( map_of_masters_t::const_iterator master_it (m_masters.begin())
          ; master_it != m_masters.end()
          ; ++master_it
          )
      {
        boost::this_thread::interruption_point();

        master_ptr master (master_it->second);
        if (master->is_connected() && master->is_polling())
        {
          at_least_one_connected = true;

          const boost::posix_time::ptime time_of_next_request
            (master->last_job_rqst() + master->cur_poll_interval());

          if (now >= time_of_next_request)
          {
            DMLOG(TRACE, "requesting job from " << master->name());

            send_event(new sdpa::events::RequestJobEvent( m_my_name
                                                        , master->name()
                                                        )
                      );

            master->job_requested();
            master->update_send();
          }

          boost::posix_time::time_duration delta_to_next_request
            (time_of_next_request - now);
          if (delta_to_next_request < min_sleep_time )
          {
            min_sleep_time = delta_to_next_request;
          }
        }
      }

      if (! at_least_one_connected)
      {
        std::string m;
        MLOG(INFO, "no body is connected, going to sleep...");
        m_connected_event.wait(m);
        MLOG(INFO, "starting to request jobs...");
      }
      else
      {
        // job arrived and computed?
        lock_type lock(m_job_arrived_mutex);
        m_job_arrived.timed_wait ( lock
                                 , now + min_sleep_time
                                 )
                                 ;
      }
    }
  }

  void job_execution_thread ()
  {
    wfe::meta_data_t meta_data;
    meta_data["agent.name"] = m_my_name;
    meta_data["agent.pid"]  = boost::lexical_cast<std::string>(getpid());
    meta_data["agent.host"] = boost::asio::ip::host_name();

    for (;;)
    {
      if (m_graceful_shutdown_requested)
      {
        fhg_kernel ()->shutdown ();
        break;
      }

      { lock_type lck (m_job_in_progress_mutex); m_job_in_progress = false; }
      drts_on_cancel_clear ();

      job_ptr_t job = m_pending_jobs.get();

      { lock_type lck (m_job_in_progress_mutex); m_job_in_progress = true; }

      if (drts::Job::PENDING == job->cmp_and_swp_state( drts::Job::PENDING
                                                      , drts::Job::RUNNING
                                                      )
         )
      {
        try
        {
          job->started(boost::posix_time::microsec_clock::universal_time());

          MLOG(TRACE, "executing job " << job->id());

          std::string result;
          std::string error_message;
          int ec = m_wfe->execute ( job->id()
                                  , job->description()
                                  , std::map<std::string, fhg::plugin::Capability*>()
                                  , result
                                  , error_message
                                  , job->worker_list ()
                                  , meta_data
                                  );
          job->set_result (result);
          job->set_result_code (ec);
          job->set_message (error_message);

          job->completed(boost::posix_time::microsec_clock::universal_time());

          MLOG( TRACE
              , "job returned."
              << " error-code := " << job->result_code()
              << " error-message := " << job->message()
              << " total-time := " << (job->completed() - job->started())
              );

          if (fhg::error::NO_ERROR == ec)
          {
            job->set_state (drts::Job::FINISHED);
          }
          else if (fhg::error::EXECUTION_CANCELLED == ec)
          {
            job->set_state (drts::Job::CANCELED);
          }
          else
          {
            job->set_state (drts::Job::FAILED);
          }
        }
        catch (std::exception const & ex)
        {
          MLOG( ERROR
              , "unexpected exception during job execution: " << ex.what()
              );
          job->set_state (drts::Job::FAILED);

          job->set_result (job->description());
          job->set_result_code (fhg::error::UNEXPECTED_ERROR);
          job->set_message (ex.what());
        }

        send_job_result_to_master (job);

        if (m_terminate_on_failure && job->state() == drts::Job::FAILED)
        {
          MLOG( WARN, "execution of job failed"
              << " and terminate on failure policy is in place."
              << " Good bye cruel world."
              );
          fhg_kernel()->terminate();
        }

        {
          lock_type lock(m_job_computed_mutex);
          m_job_computed.notify_one();
        }
      }
      else
      {
        lock_type job_map_lock (m_job_map_mutex);
        map_of_jobs_t::iterator job_it (m_jobs.find(job->id()));
        if (job_it != m_jobs.end())
        {
          DMLOG(TRACE, "ignoring and erasing non-pending job " << job->id());
          m_jobs.erase(job_it);

          fhg_kernel()->storage()->save("jobs", m_jobs);
        }
      }
    }
  }

  void service_capability_add ( std::string const &dst
                              , gspc::net::frame const &rqst
                              , gspc::net::user_ptr user
                              )
  {
    gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

    std::string virtual_capabilities (rqst.get_body ());
    std::list<std::string> capability_list;
    fhg::util::split( virtual_capabilities
                    , ","
                    , std::back_inserter(capability_list)
                    );

    BOOST_FOREACH (std::string const & cap, capability_list)
    {
      if (m_virtual_capabilities.find(cap) == m_virtual_capabilities.end())
      {
        DMLOG(TRACE, "adding virtual capability: " << cap);
        m_virtual_capabilities.insert
          (std::make_pair ( cap
                          , std::make_pair ( sdpa::Capability (cap
                                                              , "virtual"
                                                              , m_my_name
                                                              )
                                           , new fhg::plugin::Capability( cap
                                                                        , "virtual"
                                                                        )
                                           )
                          )
          );
      }
    }

    user->deliver (rply);
  }

  void service_capability_del ( std::string const &dst
                              , gspc::net::frame const &rqst
                              , gspc::net::user_ptr user
                              )
  {
    gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

    std::string virtual_capabilities (rqst.get_body ());
    std::list<std::string> capability_list;
    fhg::util::split( virtual_capabilities
                    , ","
                    , std::back_inserter(capability_list)
                    );

    BOOST_FOREACH (std::string const & cap, capability_list)
    {
      typedef map_of_capabilities_t::iterator cap_it_t;
      cap_it_t cap_it = m_virtual_capabilities.find (cap);
      if (cap_it != m_virtual_capabilities.end ())
      {
        for ( map_of_masters_t::const_iterator master_it(m_masters.begin())
            ; master_it != m_masters.end()
            ; ++master_it
            )
        {
          if (not master_it->second->is_connected())
            continue;

          send_event
            (new sdpa::events::CapabilitiesLostEvent( m_my_name
                                                    , master_it->first
                                                    , cap_it->second.first
                                                    )
          );
        }

        delete cap_it->second.second;
        m_virtual_capabilities.erase (cap);
      }
    }

    user->deliver (rply);
  }

  void service_capability_get ( std::string const &dst
                              , gspc::net::frame const &rqst
                              , gspc::net::user_ptr user
                              )
  {
    gspc::net::frame rply = gspc::net::make::reply_frame (rqst);

    typedef map_of_capabilities_t::const_iterator const_cap_it_t;
    for ( const_cap_it_t cap_it (m_capabilities.begin())
        ; cap_it != m_capabilities.end()
        ; ++cap_it
        )
    {
      rply.add_body (cap_it->first + "\n");
    }

    for ( const_cap_it_t cap_it(m_virtual_capabilities.begin())
        ; cap_it != m_virtual_capabilities.end()
        ; ++cap_it
        )
    {
      rply.add_body (cap_it->first + "\n");
    }

    user->deliver (rply);
  }

  void notify_capabilities_to_master (master_ptr const &master)
  {
    sdpa::capabilities_set_t caps;
    lock_type capabilities_lock(m_capabilities_mutex);

    typedef map_of_capabilities_t::const_iterator const_cap_it_t;
    for ( const_cap_it_t cap_it(m_capabilities.begin())
        ; cap_it != m_capabilities.end()
        ; ++cap_it
        )
    {
      caps.insert (cap_it->second.first);
    }

    for ( const_cap_it_t cap_it(m_virtual_capabilities.begin())
        ; cap_it != m_virtual_capabilities.end()
        ; ++cap_it
        )
    {
      caps.insert (cap_it->second.first);
    }

    if (! caps.empty())
    {
      send_event(new sdpa::events::CapabilitiesGainedEvent( m_my_name
                                                          , master->name()
                                                          , caps
                                                          )
                );
    }
  }

  void restore_jobs()
  {
    map_of_jobs_t old_jobs;
    fhg_kernel()->storage()->load("jobs", old_jobs);
    for ( map_of_jobs_t::iterator it (old_jobs.begin()), end (old_jobs.end())
        ; it != end
        ; ++it
        )
    {
      job_ptr_t job (it->second);

      switch (job->state())
      {
      case drts::Job::FINISHED:
        {
          lock_type job_map_lock (m_job_map_mutex);
          MLOG(INFO, "restoring information of finished job: " << job->id());
          m_jobs[it->first] = job;
        }
        break;
      case drts::Job::FAILED:
        {
          lock_type job_map_lock (m_job_map_mutex);
          MLOG(INFO, "restoring information of failed job: " << job->id());
          m_jobs[it->first] = job;
        }
        break;
      case drts::Job::PENDING:
        MLOG(WARN, "ignoring old pending job: " << job->id());
        break;
      case drts::Job::RUNNING:
        MLOG(WARN, "ignoring old running job: " << job->id());
        break;
      case drts::Job::CANCELED:
        MLOG(WARN, "ignoring old canceled job: " << job->id());
        break;
      default:
        MLOG(ERROR, "STRANGE job state: " << job->state());
        break;
      }
    }
  }

  void resend_outstanding_events (master_ptr const &master)
  {
    MLOG(TRACE, "resending outstanding notifications to " << master->name());
    lock_type job_map_lock (m_job_map_mutex);
    for ( map_of_jobs_t::iterator job_it (m_jobs.begin()), end (m_jobs.end())
        ; job_it != end
        ; ++job_it
        )
    {
      job_ptr_t job (job_it->second);
      MLOG( TRACE
          , "checking job"
          << " id := " << job->id()
          << " state := " << job->state()
          << " owner := " << job->owner()
          );
      if (   (job->owner() == master->name())
         && (job->state() >= drts::Job::FINISHED)
         )
      {
        MLOG(TRACE, "resending outcome of job " << job->id());
        send_job_result_to_master (job);
      }
    }
  }

  int send_job_result_to_master (job_ptr_t const & job)
  {
    switch (job->state())
    {
    case drts::Job::FINISHED:
      return send_event (new sdpa::events::JobFinishedEvent ( m_my_name
                                                            , job->owner()
                                                            , job->id()
                                                            , job->result()
                                                            )
                        );
      break;
    case drts::Job::FAILED:
      {
        return send_event
          (new sdpa::events::JobFailedEvent ( m_my_name
                                            , job->owner()
                                            , job->id()
                                            , job->result()
                                            , job->result_code()
                                            , job->message()
                                            )
          );
      }
      break;
    case drts::Job::CANCELED:
      {
        return send_event
          (new sdpa::events::CancelJobAckEvent ( m_my_name
                                               , job->owner()
                                               , job->id()
                                               , job->result()
                                               )
          );
      }
      break;
    default:
      return -EINVAL;
    }
  }

  master_ptr create_master (std::string master)
  {
    if (master.empty())
    {
      throw std::runtime_error ("empty master specified!");
    }

    bool polling (false);
    if (master[0] == '+')
    {
      polling = true;
      master = master.substr(1);
    }

    if (master.empty())
    {
      throw std::runtime_error ("polling master with empty name specified!");
    }

    if (master == m_my_name)
    {
      throw std::runtime_error ("cannot be my own master!");
    }
    master_ptr m (new drts::Master(master));
    m->set_is_polling (polling);
    return m;
  }

  void start_connect ()
  {
    bool at_least_one_disconnected = false;
    bool not_connected_to_anyone = true;

    for ( map_of_masters_t::iterator master_it (m_masters.begin())
        ; master_it != m_masters.end()
        ; ++master_it
        )
    {
      boost::shared_ptr<drts::Master> master (master_it->second);

      if (! master->is_connected())
      {
        sdpa::events::WorkerRegistrationEvent::Ptr evt
          (new sdpa::events::WorkerRegistrationEvent( m_my_name
                                                    , master->name()
                                                    , m_backlog_size
                                                    )
          );

        send_event(evt);

        master->update_send();

        at_least_one_disconnected = true;
      }
      else
      {
        not_connected_to_anyone = false;
      }
    }

    if (not_connected_to_anyone)
    {
      if (m_max_reconnect_attempts)
      {
        lock_type lock_reconnect_rounter (m_reconnect_counter_mutex);
        if (m_reconnect_counter < m_max_reconnect_attempts)
        {
          ++m_reconnect_counter;
        }
        else
        {
          MLOG( WARN
              , "still not connected after " << m_reconnect_counter
              << " trials: shutting down"
              );
          fhg_kernel()->shutdown();
          return;
        }
      }
    }


    if (at_least_one_disconnected)
    {
      fhg_kernel()->schedule ( "connect"
                             , boost::bind( &DRTSImpl::start_connect
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
        MLOG(WARN, "could not handle incoming message: " << ex.what());
      }
      start_receiver();
    }
    else if (! m_shutting_down)
    {
      const fhg::com::p2p::address_t & addr = m_message.header.src;
      if (addr != m_peer->address())
      {
        const std::string other_name(m_peer->resolve (addr, "*unknown*"));

        map_of_masters_t::iterator master(m_masters.find(other_name));
        if (master != m_masters.end() && master->second->is_connected())
        {
          DMLOG ( INFO
                , "connection to " << other_name << " lost: " << ec.message()
                );

          master->second->is_connected(false);

          fhg_kernel()->schedule ( "connect"
                                 , boost::bind( &DRTSImpl::start_connect
                                              , this
                                              )
                                 , 5
                                 );
        }

        start_receiver();
      }
      else
      {
        MLOG(TRACE, m_peer->name() << " is shutting down");
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
      DMLOG_EVERY_N( WARN, 10, "could not send "
                   << evt->str() << " to " << evt->to() << ": " << ex.what()
                   );
      return -ESRCH;
    }

    return 0;
  }

  void dispatch_event (sdpa::events::SDPAEvent::Ptr const &evt)
  {
    if (evt)
    {
      DMLOG(TRACE, "received event: " << evt->str());
      m_event_queue.put(evt);
    }
    else
    {
      MLOG(WARN, "got invalid message from suspicious source");
    }
  }

  bool m_shutting_down;
  bool m_terminate_on_failure;

  wfe::WFE *m_wfe;

  boost::shared_ptr<boost::thread>    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  std::string m_my_name;
  map_of_masters_t m_masters;
  std::size_t m_max_reconnect_attempts;
  std::size_t m_reconnect_counter;

  event_queue_t m_event_queue;
  boost::shared_ptr<boost::thread>    m_event_thread;
  boost::shared_ptr<boost::thread>    m_request_thread;
  boost::shared_ptr<boost::thread>    m_execution_thread;

  mutable mutex_type m_job_map_mutex;
  mutable mutex_type m_job_computed_mutex;
  condition_type     m_job_computed;
  mutable mutex_type m_job_arrived_mutex;
  mutable mutex_type m_reconnect_counter_mutex;
  condition_type     m_job_arrived;
  mutable mutex_type m_job_in_progress_mutex;
  bool               m_job_in_progress;
  bool               m_graceful_shutdown_requested;

  fhg::util::thread::event<std::string> m_connected_event;

  mutable mutex_type m_capabilities_mutex;
  map_of_capabilities_t m_capabilities;
  map_of_capabilities_t m_virtual_capabilities;

  // jobs + their states
  size_t m_backlog_size;
  map_of_jobs_t m_jobs;
  job_queue_t m_pending_jobs;
};

EXPORT_FHG_PLUGIN( drts
                 , DRTSImpl
                 , "DRTS"
                 , "provides access to the distributed runtime-system"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "kvs,wfe"
                 , ""
                 );
