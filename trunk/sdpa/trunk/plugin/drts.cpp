#include "drts.hpp"
#include "master.hpp"
#include "job.hpp"
#include "wfe.hpp"
#include "observable.hpp"

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

#include <sdpa/uuidgen.hpp>
#include <sdpa/events/EventHandler.hpp>
#include <sdpa/events/Codec.hpp>
#include <sdpa/events/events.hpp>

#include <fhgcom/peer.hpp>

#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <sdpa/types.hpp>

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

  typedef std::map<std::string, fhg::plugin::Capability*> map_of_capabilities_t;
public:
  FHG_PLUGIN_START()
  {
    m_shutting_down = false;
    m_my_name =      fhg_kernel()->get("name", "drts");
    try
    {
      m_backlog_size = fhg_kernel()->get<size_t>("backlog", "3");
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not parse backlog size: " << fhg_kernel()->get("backlog", "3") << ": " << ex.what());
      FHG_PLUGIN_FAILED(EINVAL);
    }

    m_wfe = fhg_kernel()->acquire<wfe::WFE>("wfe");
    if (0 == m_wfe)
    {
      MLOG(ERROR, "could not access workflow-engine plugin!");
      FHG_PLUGIN_FAILED(ELIBACC);
    }

    // TODO: add
    //
    //      acquire_all<T>() -> [(name, T*)]
    //
    // to get access to all plugins of a particular type
    if (fhg::plugin::Capability* cap = fhg_kernel()->acquire<fhg::plugin::Capability>("gpi"))
    {
      MLOG(INFO, "gained capability: " << cap->capability_name() << " of type " << cap->capability_type());

      lock_type cap_lock(m_capabilities_mutex);
      m_capabilities.insert (std::make_pair(cap->capability_name(), cap));
    }

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
            MLOG(INFO, "adding master \"" << m->name() << "\"");
            m_masters.insert (std::make_pair(m->name(), m));

            have_master_with_polling |= m->is_polling();
          }
          else
          {
            MLOG(WARN, "master already specified, ignoring new one: " << master);
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

    if (have_master_with_polling)
      m_request_thread.reset(new boost::thread(&DRTSImpl::job_requestor_thread, this));

    m_execution_thread.reset(new boost::thread(&DRTSImpl::job_execution_thread, this));

    start_connect ();

    FHG_PLUGIN_STARTED();
  }

  FHG_PLUGIN_STOP()
  {
    // cancel running jobs etc.
    m_shutting_down = true;

    if (m_request_thread)
    {
      m_request_thread->interrupt();
      m_request_thread->join();
      m_request_thread.reset();
    }

    if (m_execution_thread)
    {
      m_execution_thread->interrupt();
      m_execution_thread->join ();
      m_execution_thread.reset();
    }

    // cancel all pending jobs
    //    try to deliver outstanding notifications
    //    dump state of job-map

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

    m_wfe = 0;

    FHG_PLUGIN_STOPPED();
  }

  FHG_ON_PLUGIN_LOADED(plugin)
  {
    if (fhg::plugin::Capability* cap = fhg_kernel()->acquire<fhg::plugin::Capability>(plugin))
    {
      MLOG(INFO, "gained capability: " << cap->capability_name() << " of type " << cap->capability_type());

      lock_type cap_lock(m_capabilities_mutex);
      m_capabilities.insert (std::make_pair(cap->capability_name(), cap));

      for ( map_of_masters_t::const_iterator master_it(m_masters.begin())
          ; master_it != m_masters.end()
          ; ++master_it
          )
      {
        if (master_it->second->is_connected())
        {
        	sdpa::capability_t sdpa_cap(cap->capability_name(), cap->capability_type());
        	send_event (new sdpa::events::CapabilitiesGainedEvent( m_my_name
                                                               , master_it->first
                                                               , sdpa_cap
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
      m_capabilities.erase(cap);

      for ( map_of_masters_t::const_iterator master_it(m_masters.begin())
          ; master_it != m_masters.end()
          ; ++master_it
          )
      {
        if (master_it->second->is_connected())
          send_event (new sdpa::events::CapabilitiesLostEvent( m_my_name
                                                             , master_it->first
                                                             , cap->first
                                                             )
                     );
      }
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
  virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent *e)
  {
    map_of_masters_t::iterator master_it (m_masters.find(e->from()));
    if (master_it != m_masters.end() && !master_it->second->is_connected())
    {
      MLOG(INFO, "successfully connected to " << master_it->second->name());
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
  }
  virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent *e)
  {
    MLOG(WARN, "worker tried to register: " << e->from());

    send_event (new sdpa::events::ErrorEvent( m_my_name
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

  virtual void handleConfigNokEvent(const sdpa::events::ConfigNokEvent *)
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
    send_event (new sdpa::events::ErrorEvent( m_my_name
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
      send_event (new sdpa::events::ErrorEvent( m_my_name
                                              , e->from()
                                              , sdpa::events::ErrorEvent::SDPA_EPERM
                                              , "you are not yet connected"
                                              )
                 );
      return;
    }

    job_ptr_t job (new drts::Job( drts::Job::ID(e->job_id())
                                , drts::Job::Description(e->description())
                                , drts::Job::Owner(e->from())
                                )
                  );

    {
      lock_type job_map_lock(m_job_map_mutex);

      if (m_backlog_size && m_pending_jobs.size() >= m_backlog_size)
      {
        MLOG(WARN, "cannot accept new job (" << job->id() << "), backlog is full.");
        send_event (new sdpa::events::ErrorEvent( m_my_name
                                                , e->from()
                                                , sdpa::events::ErrorEvent::SDPA_EJOBREJECTED
                                                , e->job_id() // "I am busy right now, please try again later"
                                                )
                   );
        return;
      }
      else
      {
        send_event (new sdpa::events::SubmitJobAckEvent( m_my_name
                                                       , job->owner()
                                                       , job->id()
                                                       , "empty-message-id"
                                                       )
                   );
        master->second->reset_poll_rate();
        m_jobs.insert (std::make_pair(job->id(), job));

        job->entered(boost::posix_time::microsec_clock::universal_time());

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
      MLOG(ERROR, "could not cancel job: " << e->job_id() << ": not found");
      send_event (new sdpa::events::ErrorEvent( m_my_name
                                              , e->from()
                                              , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                                              , "could not find job " + std::string(e->job_id())
                                              )
                 );
    }
    else if (job_it->second->owner() != e->from())
    {
      MLOG(ERROR, "could not cancel job: " << e->job_id() << ": not owner");
      send_event (new sdpa::events::ErrorEvent( m_my_name
                                              , e->from()
                                              , sdpa::events::ErrorEvent::SDPA_EPERM
                                              , "you are not the owner of job " + std::string(e->job_id())
                                              )
                 );
      return;
    }
    else
    {
      if (drts::Job::PENDING == job_it->second->cmp_and_swp_state( drts::Job::PENDING
                                                                 , drts::Job::CANCELED
                                                                 )
         )
      {
        MLOG(TRACE, "cancelling pending job: " << e->job_id());
        send_event (new sdpa::events::CancelJobAckEvent ( m_my_name
                                                        , job_it->second->owner()
                                                        , job_it->second->id()
                                                        , "canceled"
                                                        )
                   );
      }
      else if (job_it->second->state() == drts::Job::RUNNING)
      {
        MLOG(TRACE, "trying to cancel running job " << e->job_id());
        m_wfe->cancel (e->job_id());
      }
      else
      {
        MLOG(WARN, "what shall I do with an already computed job? (" << e->job_id() << ")");
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
      MLOG(ERROR, "could not acknowledge failed job: " << e->job_id() << ": not found");
      send_event (new sdpa::events::ErrorEvent( m_my_name
                                              , e->from()
                                              , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                                              , "could not find job " + std::string(e->job_id())
                                              )
                 );
      return;
    }
    else if (job_it->second->owner() != e->from())
    {
      MLOG(ERROR, "could not acknowledge failed job: " << e->job_id() << ": not owner");
      send_event (new sdpa::events::ErrorEvent( m_my_name
                                              , e->from()
                                              , sdpa::events::ErrorEvent::SDPA_EPERM
                                              , "you are not the owner of job " + std::string(e->job_id())
                                              )
                 );
      return;
    }

    DMLOG(TRACE, "removing job " << e->job_id());
    m_jobs.erase (job_it);
  }

  virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent *e)
  {
    // locate the job
    lock_type job_map_lock (m_job_map_mutex);
    map_of_jobs_t::iterator job_it (m_jobs.find(e->job_id()));
    if (job_it == m_jobs.end())
    {
      MLOG(ERROR, "could not acknowledge finished job: " << e->job_id() << ": not found");
      send_event (new sdpa::events::ErrorEvent( m_my_name
                                              , e->from()
                                              , sdpa::events::ErrorEvent::SDPA_EJOBNOTFOUND
                                              , "could not find job " + std::string(e->job_id())
                                              )
                 );
      return;
    }
    else if (job_it->second->owner() != e->from())
    {
      MLOG(ERROR, "could not acknowledge finished job: " << e->job_id() << ": not owner");
      send_event (new sdpa::events::ErrorEvent( m_my_name
                                              , e->from()
                                              , sdpa::events::ErrorEvent::SDPA_EPERM
                                              , "you are not the owner of job " + std::string(e->job_id())
                                              )
                 );
      return;
    }

    DMLOG(TRACE, "removing job " << e->job_id());
    m_jobs.erase (job_it);
  }

  // not implemented events
  virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent *){}
  virtual void handleConfigOkEvent(const sdpa::events::ConfigOkEvent *) {}
  virtual void handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent *) {}
  virtual void handleConfigRequestEvent(const sdpa::events::ConfigRequestEvent *) {}
  virtual void handleDeleteJobAckEvent(const sdpa::events::DeleteJobAckEvent *) {}
  virtual void handleInterruptEvent(const sdpa::events::InterruptEvent *){}
  virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent *) {}
  virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent *) {}
  virtual void handleJobResultsReplyEvent(const sdpa::events::JobResultsReplyEvent *) {}
  virtual void handleJobStatusReplyEvent(const sdpa::events::JobStatusReplyEvent *) {}
  virtual void handleLifeSignEvent(const sdpa::events::LifeSignEvent *) {}
  virtual void handleRunJobEvent(const sdpa::events::RunJobEvent *) {}
  virtual void handleStartUpEvent(const sdpa::events::StartUpEvent *) {}
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
          MLOG(TRACE, "job requestor waits until job queue frees up some slots");
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
      job_ptr_t job = m_pending_jobs.get();
      if (drts::Job::PENDING == job->cmp_and_swp_state( drts::Job::PENDING
                                                      , drts::Job::RUNNING
                                                      )
         )
      {
        try
        {
          std::string result;

          job->started(boost::posix_time::microsec_clock::universal_time());

          MLOG(TRACE, "executing job " << job->id());

          int ec = m_wfe->execute ( job->id()
                                  , job->description()
                                  , m_capabilities
                                  , result
                                  , meta_data
                                  );

          job->completed(boost::posix_time::microsec_clock::universal_time());

          MLOG( TRACE
              , "job returned."
              << " error-code := " << ec
              << " total-time := " << (job->completed() - job->started())
              );

          if (ec > 0)
          {
            job->cmp_and_swp_state (drts::Job::RUNNING, drts::Job::FAILED);
            ensure_send_event (new sdpa::events::JobFailedEvent ( m_my_name
                                                                , job->owner()
                                                                , job->id()
                                                                , result
                                                                )
                              );
          }
          else if (ec < 0)
          {
            job->cmp_and_swp_state (drts::Job::RUNNING, drts::Job::CANCELED);
            ensure_send_event (new sdpa::events::CancelJobAckEvent ( m_my_name
                                                                   , job->owner()
                                                                   , job->id()
                                                                   , result
                                                                   )
                              );

            lock_type job_map_lock (m_job_map_mutex);
            map_of_jobs_t::iterator job_it (m_jobs.find(job->id()));
            assert (job_it != m_jobs.end());
            DMLOG(TRACE, "removing job " << job->id());
            m_jobs.erase(job_it);
          }
          else
          {
            job->cmp_and_swp_state (drts::Job::RUNNING, drts::Job::FINISHED);
            ensure_send_event (new sdpa::events::JobFinishedEvent ( m_my_name
                                                                  , job->owner()
                                                                  , job->id()
                                                                  , result
                                                                  )
                              );
          }
        }
        catch (std::exception const & ex)
        {
          // mark job as failed
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
        assert (job_it != m_jobs.end());
        DMLOG(TRACE, "ignoring and erasing non-pending job " << job->id());
        m_jobs.erase(job_it);
      }
    }
  }

  void notify_capabilities_to_master (master_ptr const &master)
  {
    sdpa::capabilities_set_t caps;
    lock_type capabilities_lock(m_capabilities_mutex);
    for ( map_of_capabilities_t::const_iterator cap_it(m_capabilities.begin())
        ; cap_it != m_capabilities.end()
        ; ++cap_it
        )
    {
      caps.insert (cap_it->first);
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

  void resend_outstanding_events (master_ptr const &master)
  {
    MLOG(TRACE, "resending outstanding events to " << master->name());
    while (master->outstanding_events().size() && master->is_connected())
    {
      sdpa::events::SDPAEvent::Ptr evt(master->outstanding_events().get());
      ensure_send_event (evt);
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
    }

    if (at_least_one_disconnected)
    {
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
          MLOG(WARN, "connection to " << other_name << " lost: " << ec);

          master->second->is_connected(false);

          fhg_kernel()->schedule (boost::bind( &DRTSImpl::start_connect
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

  int ensure_send_event (sdpa::events::SDPAEvent *e)
  {
    return ensure_send_event (sdpa::events::SDPAEvent::Ptr(e));
  }

  int ensure_send_event (sdpa::events::SDPAEvent::Ptr const & evt)
  {
    int ec = send_event (evt);

    if (0 != ec)
    {
      // check master
      map_of_masters_t::iterator master (m_masters.find(evt->to()));
      if (master != m_masters.end())
      {
        master->second->outstanding_events().put(evt);
      }
      else
      {
        return ec;
      }
    }
    return 0;
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
      MLOG(WARN, "could not send " << evt->str() << " to " << evt->to() << ": " << ex.what());
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

  wfe::WFE *m_wfe;

  boost::shared_ptr<boost::thread>    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  std::string m_my_name;
  map_of_masters_t m_masters;

  event_queue_t m_event_queue;
  boost::shared_ptr<boost::thread>    m_event_thread;
  boost::shared_ptr<boost::thread>    m_request_thread;
  boost::shared_ptr<boost::thread>    m_execution_thread;

  mutable mutex_type m_job_map_mutex;
  mutable mutex_type m_job_computed_mutex;
  condition_type     m_job_computed;
  mutable mutex_type m_job_arrived_mutex;
  condition_type     m_job_arrived;

  fhg::util::thread::event<std::string> m_connected_event;

  mutable mutex_type m_capabilities_mutex;
  map_of_capabilities_t m_capabilities;

  // jobs + their states
  size_t m_backlog_size;
  map_of_jobs_t m_jobs;
  job_queue_t m_pending_jobs;
};

EXPORT_FHG_PLUGIN( drts
                 , DRTSImpl
                 , "provides access to the distributed runtime-system"
                 , "Alexander Petry <petry@itwm.fhg.de>"
                 , "0.0.1"
                 , "NA"
                 , "kvs,wfe"
                 , ""
                 );
