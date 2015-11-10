/*
 * =====================================================================================
 *
 *       Filename:  GenericDaemon.cpp
 *
 *    Description:  Generic daemon implementation file
 *
 *        Version:  1.0
 *        Created:  2009
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>
#include <sdpa/events/delayed_function_call.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/events/put_token.hpp>
#include <sdpa/id_generator.hpp>

#include <fhg/util/boost/optional.hpp>
#include <util-generic/hostname.hpp>
#include <fhg/util/macros.hpp>
#include <util-generic/join.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/tokenizer.hpp>
#include <boost/range/adaptor/map.hpp>

#include <functional>
#include <sstream>
#include <algorithm>
#include <thread>
#include <chrono>

namespace sdpa
{
  struct opaque_job_master_t::implementation
  {
    using actual_implementation =
      boost::optional<daemon::GenericDaemon::master_info_t::iterator>;

    implementation (actual_implementation actual)
      : _actual (std::move (actual))
    {}
    actual_implementation const _actual;
  };
  opaque_job_master_t::opaque_job_master_t (opaque_job_master_t&& rhs)
    : _ (rhs._)
  {
    rhs._ = nullptr;
  }
  opaque_job_master_t::~opaque_job_master_t()
  {
    delete _;
    _ = nullptr;
  }
  opaque_job_master_t::opaque_job_master_t (const void* data)
    : _ ( new implementation
          (*static_cast<const implementation::actual_implementation*> (data))
        )
  {}

  namespace daemon {
namespace
{
  std::vector<std::string> require_proper_url (std::string url)
  {
    const boost::tokenizer<boost::char_separator<char>> tok
      (url, boost::char_separator<char> (":"));

    const std::vector<std::string> vec (tok.begin(), tok.end());

    if (vec.empty() || vec.size() > 2)
    {
      throw std::runtime_error ("configuration of network failed: invalid url: has to be of format 'host[:port]'");
    }

    return vec;
  }

  fhg::com::host_t host_from_url (std::string url)
  {
    return fhg::com::host_t (require_proper_url (url)[0]);
  }
  fhg::com::port_t port_from_url (std::string url)
  {
    const std::vector<std::string> vec (require_proper_url (url));
    return fhg::com::port_t (vec.size() == 2 ? vec[1] : "0");
  }
}

    GenericDaemon::virtual_memory_api::virtual_memory_api
        ( fhg::log::Logger& logger
        , boost::filesystem::path const& socket
        )
      : _ (logger, socket.string())
    {}

    GenericDaemon::master_network_info::master_network_info
        (std::string const& host_, std::string const& port_)
      : host (host_)
      , port (port_)
      , address (boost::none)
    {}

    namespace
    {
      //! \note templated for convenience of not needing public access
      //! to master info stuff
      template<typename InfoMap> InfoMap make_master_info_map
        (std::vector<name_host_port_tuple> const& masters)
      {
        InfoMap ret;
        for (name_host_port_tuple const& name_host_port : masters)
        {
          ret.emplace ( std::get<0> (name_host_port)
                      , typename InfoMap::mapped_type
                          ( std::get<1> (name_host_port)
                          , std::get<2> (name_host_port)
                          )
                      );
        }
        return ret;
      }
    }

GenericDaemon::GenericDaemon( const std::string name
                            , const std::string url
                            , std::unique_ptr<boost::asio::io_service> peer_io_service
                            , boost::optional<boost::filesystem::path> const& vmem_socket
                            , std::vector<name_host_port_tuple> const& masters
                            , fhg::log::Logger& logger
                            , const boost::optional<std::pair<std::string, boost::asio::io_service&>>& gui_info
                            , bool create_wfe
                            )
  : _logger (logger)
  , _name (name)
  , _master_info (make_master_info_map<master_info_t> (masters))
  , _subscriptions()
  , _discover_sources()
  , _job_map_mutex()
  , job_map_()
  , _cleanup_job_map_on_dtor_helper (job_map_)
  , _worker_manager()
  , _scheduler ( [this] (job_id_t job_id)
                 {
                   return findJob (job_id)->requirements();
                 }
               , _worker_manager
               )
  , _scheduling_thread_mutex()
  , _scheduling_thread_notifier()
  , _random_extraction_engine
    (boost::make_optional (create_wfe, std::mt19937 (std::random_device()())))
  , mtx_subscriber_()
  , mtx_cpb_()
  , m_capabilities()
  , m_guiService ( gui_info && !gui_info->first.empty()
                 ? fhg::util::cxx14::make_unique<NotificationService>
                   (gui_info->first, gui_info->second)
                 : nullptr
                 )
  , _registration_timeout (boost::posix_time::seconds (1))
  , _event_queue()
  , _network_strategy ( [this] ( fhg::com::p2p::address_t const& source
                               , events::SDPAEvent::Ptr const& e
                               )
                      {
                        _event_queue.put (source, e);
                      }
                      , std::move (peer_io_service)
                      , host_from_url (url)
                      , port_from_url (url)
                      )
  , ptr_workflow_engine_ ( create_wfe
                         ? new we::layer
                           ( std::bind (&GenericDaemon::submit, this, std::placeholders::_1, std::placeholders::_2)
                           , std::bind (&GenericDaemon::cancel, this, std::placeholders::_1)
                           , std::bind (&GenericDaemon::finished, this, std::placeholders::_1, std::placeholders::_2)
                           , std::bind (&GenericDaemon::failed, this, std::placeholders::_1, std::placeholders::_2)
                           , std::bind (&GenericDaemon::canceled, this, std::placeholders::_1)
                           , std::bind (&GenericDaemon::discover, this, std::placeholders::_1, std::placeholders::_2)
                           , std::bind (&GenericDaemon::discovered, this, std::placeholders::_1, std::placeholders::_2)
                           , std::bind (&GenericDaemon::token_put, this, std::placeholders::_1, std::placeholders::_2)
                           , std::bind (&GenericDaemon::workflow_response_response, this, std::placeholders::_1, std::placeholders::_2)
                           , std::bind (&GenericDaemon::gen_id, this)
                           , *_random_extraction_engine
                           )
                         : nullptr
                         )
  , _registration_threads()
  , _scheduling_thread (&GenericDaemon::scheduling_thread, this)
  , _event_handler_thread (&GenericDaemon::handle_events, this)
  , _virtual_memory_api
    ( vmem_socket
    ? fhg::util::cxx14::make_unique<virtual_memory_api>
        (_logger, *vmem_socket)
    : nullptr
    )
{
    for ( master_info_t::iterator it (_master_info.begin())
        ; it != _master_info.end()
        ; ++it
        )
    {
      requestRegistration (it);
    }
}

GenericDaemon::cleanup_job_map_on_dtor_helper::cleanup_job_map_on_dtor_helper
    (job_map_t& m)
  : _ (m)
{}
GenericDaemon::cleanup_job_map_on_dtor_helper::~cleanup_job_map_on_dtor_helper()
{
  for (const Job* const pJob : _ | boost::adaptors::map_values )
  {
    delete pJob;
  }
}

//! \todo Move to gpi::pc::client::api_t
std::function<double (std::string const&)>
  GenericDaemon::virtual_memory_api::transfer_costs (const we::type::activity_t& activity)
{
  if (!activity.transition().module_call())
    return null_transfer_cost;

  expr::eval::context context;

  for ( std::pair< pnet::type::value::value_type
                , we::port_id_type
                > const& token_on_port
      : activity.input()
      )
  {
   context.bind_ref
     ( activity.transition().ports_input().at (token_on_port.second).name()
     , token_on_port.first
     );
  }

  std::list<std::pair<we::local::range, we::global::range>>
    vm_transfers (activity.transition().module_call()->gets (context));

  std::list<std::pair<we::local::range, we::global::range>>
    puts_before (activity.transition().module_call()->puts_evaluated_before_call (context));

  vm_transfers.splice (vm_transfers.end(), puts_before);

  if (vm_transfers.empty())
  {
    return null_transfer_cost;
  }

  return _.transfer_costs (vm_transfers);
}

const std::string& GenericDaemon::name() const
{
  return _name;
}

    boost::asio::ip::tcp::endpoint GenericDaemon::peer_local_endpoint() const
    {
      return _network_strategy.local_endpoint();
    }

void GenericDaemon::serveJob(std::set<worker_id_t> const& workers, const job_id_t& jobId)
{
  //take a job from the workers' queue and serve it
  Job* ptrJob = findJob(jobId);
  if(ptrJob)
  {
      // create a SubmitJobEvent for the job job_id serialize and attach description
      LLOG(TRACE, _logger, "The job "<<ptrJob->id()<<" was assigned the following workers: {"<< fhg::util::join (workers, ", ") << '}');

      for (const worker_id_t& worker_id : workers)
      {
        child_proxy (this, _worker_manager.address_by_worker (worker_id).get()->second)
          .submit_job (ptrJob->id(), ptrJob->description(), workers);
      }
  }
}

std::string GenericDaemon::gen_id()
{
  static id_generator generator ("job");
  return generator.next();
}

    Job* GenericDaemon::addJob ( const sdpa::job_id_t& job_id
                               , const job_desc_t desc
                               , boost::optional<master_info_t::iterator> owner
                               , const job_requirements_t& job_req_list
                               )
    {
      boost::mutex::scoped_lock const _ (_job_map_mutex);

      Job* pJob = new Job( job_id, desc, opaque_job_master_t (static_cast<const void*> (&owner)), job_req_list);

      if (!job_map_.emplace (job_id, pJob).second)
      {
        delete pJob;
        throw std::runtime_error ("job with same id already exists");
      }

      return pJob;
    }

    Job* GenericDaemon::findJob(const sdpa::job_id_t& job_id ) const
    {
      boost::mutex::scoped_lock const _ (_job_map_mutex);

      const job_map_t::const_iterator it (job_map_.find( job_id ));
      return it != job_map_.end() ? it->second : nullptr;
    }
    void GenericDaemon::deleteJob(const sdpa::job_id_t& job_id)
    {
      boost::mutex::scoped_lock const _ (_job_map_mutex);

      const job_map_t::const_iterator it (job_map_.find( job_id ));
      if (it == job_map_.end())
      {
        throw std::runtime_error ("deleteJob: job not found");
      }

      delete it->second;
      job_map_.erase (it);
    }

void GenericDaemon::handleSubmitJobEvent
  (fhg::com::p2p::address_t const& source, const events::SubmitJobEvent* evt)
{
  const events::SubmitJobEvent& e (*evt);

  boost::optional<master_info_t::iterator> const itMaster
    (master_by_address (source));

  // First, check if the job 'job_id' wasn't already submitted!
  if(e.job_id() && findJob(*e.job_id()))
  {
    parent_proxy (this, source).submit_job_ack (*e.job_id());
    return;
  }

  const job_id_t job_id (e.job_id() ? *e.job_id() : job_id_t (gen_id()));

  // One should parse the workflow in order to be able to create a valid job
  Job* pJob (addJob ( job_id
                    , e.description()
                    , itMaster
                    , {{}, we::type::schedule_data(), null_transfer_cost, 1.0, 0} //!Note: a master job needs no shared mem allocation
                    )
             );

  //! \todo Don't ack before we know that we can: may fail 20 lines
  //! below. add Nack event of some sorts to not need
  //! submitack+jobfailed to fail submission. this would also resolve
  //! the race in handleDiscoverJobStatesEvent.
  parent_proxy (this, source).submit_job_ack (job_id);

  // check if the message comes from outside or from WFE
  // if it comes from outside and the agent has an WFE, submit it to it
  if(hasWorkflowEngine() )
  {
    try
    {
      const we::type::activity_t act (pJob->description());
      workflowEngine()->submit (job_id, act);

      // Should set the workflow_id here, or send it together with the workflow description
      pJob->Dispatch();

      if (m_guiService)
      {
        const sdpa::daemon::NotificationEvent evt_
          ( {name()}
          , job_id
          , NotificationEvent::STATE_STARTED
          , act
          );

        m_guiService->notify (evt_);
      }
    }
    catch (...)
    {
      fhg::util::current_exception_printer const error (": ");
      LLOG (ERROR, _logger, "Exception occurred: " << error << ". Failed to submit the job "<<job_id<<" to the workflow engine!");

      failed (job_id, error.string());
    }
  }
  else {
    _scheduler.enqueueJob(job_id);
    request_scheduling();
  }
}

void GenericDaemon::handleWorkerRegistrationEvent
  (fhg::com::p2p::address_t const& source, const events::WorkerRegistrationEvent* event)
try
{
  // check if the worker source has already registered!
  // delete inherited capabilities that are owned by the current agent
  sdpa::capabilities_set_t workerCpbSet;

  // take the difference
  for (const sdpa::capability_t& cpb : event->capabilities())
  {
    // own capabilities have always the depth 0 and are not inherited
    // by the descendants
    if (!isOwnCapability (cpb))
    {
      sdpa::capability_t cpbMod (cpb);
      cpbMod.incDepth();
      workerCpbSet.insert (cpbMod);
    }
  }

  _worker_manager.addWorker
    ( event->name()
    , workerCpbSet
    , event->allocated_shared_memory_size()
    , event->children_allowed()
    , event->hostname(), source
    );

  request_rescheduling (event->name());

  // send to the masters my new set of capabilities
  for (master_info_t::value_type const& info : _master_info)
  {
    if (info.second.address)
    {
      parent_proxy (this, *info.second.address)
        .capabilities_gained (workerCpbSet);
    }
  }

  child_proxy (this, source)
    .worker_registration_response (boost::none);
}
catch (...)
{
  child_proxy (this, source)
    .worker_registration_response (std::current_exception());
}

void GenericDaemon::handleErrorEvent
  (fhg::com::p2p::address_t const& source, const events::ErrorEvent* evt)
{
  const sdpa::events::ErrorEvent& error (*evt);

  boost::optional<master_info_t::iterator> const as_master
    (master_by_address (source));
  boost::optional<WorkerManager::worker_connections_t::right_map::iterator> const as_worker
    (_worker_manager.worker_by_address (source));

  // if it'a communication error, inspect all jobs and
  // send results if they are in a terminal state

  switch (error.error_code())
  {
    // this  should  better go  into  a  distinct  event, since  the  ErrorEvent
    // 'reason' should not be reused for important information
    case events::ErrorEvent::SDPA_EBACKLOGFULL:
    {
      sdpa::job_id_t jobId(*error.job_id());
      Job* pJob (findJob (jobId));
      if (!pJob)
      {
        throw std::runtime_error ("Got SDPA_EBACKLOGFULL error related to unknown job!");
      }

      if (!as_worker)
      {
        throw std::runtime_error ("Unknown entity (unregister worker) rejected the job " + jobId);
      }

      _worker_manager.set_worker_backlog_full (as_worker.get()->second, true);

      if (sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ("Got SDPA_EBACKLOGFULL error for an already terminated job!");
      }

      _scheduler.workerCanceled (as_worker.get()->second, jobId);
      pJob->Reschedule();

      if ( !_scheduler.cancelNotTerminatedWorkerJobs
             ( [this, &jobId](const sdpa::worker_id_t& wid)
               {
                 child_proxy (this, _worker_manager.address_by_worker (wid).get()->second)
                   .cancel_job (jobId);
               }
             , jobId
             )
         )
      {
        _scheduler.releaseReservation (jobId);
        _scheduler.enqueueJob (jobId);
        _scheduler.assignJobsToWorkers();
      }

      break;
    }
    case events::ErrorEvent::SDPA_ENODE_SHUTDOWN:
    case events::ErrorEvent::SDPA_ENETWORKFAILURE:
    {
      if( isSubscriber(source) )
        unsubscribe(source);

      if (as_worker)
      {
        for (master_info_t::value_type const& info : _master_info)
        {
          if (info.second.address)
          {
            parent_proxy (this, *info.second.address).capabilities_lost
              (_worker_manager.worker_capabilities (as_worker.get()->second));
          }
        }

        _scheduler.reschedule_pending_jobs_matching_worker (as_worker.get()->second);

        const std::set<job_id_t> jobs_to_reschedule
          (_worker_manager.get_worker_jobs_and_clean_queues (as_worker.get()->second));

        for (sdpa::job_id_t jobId : jobs_to_reschedule)
        {
          Job* pJob = findJob (jobId);
          if (pJob && !sdpa::status::is_terminal (pJob->getStatus()))
          {
            _scheduler.workerCanceled (as_worker.get()->second, jobId);
            pJob->Reschedule();

            if (!_scheduler.cancelNotTerminatedWorkerJobs
              ( [this, &jobId](const sdpa::worker_id_t& wid)
                {
                  child_proxy (this, _worker_manager.address_by_worker (wid).get()->second)
                    .cancel_job (jobId);
                }
                , jobId
                )
              )
            {
              _scheduler.releaseReservation (jobId);
              _scheduler.enqueueJob (jobId);
            }
          }
        }

        _worker_manager.deleteWorker (as_worker.get()->second);
        request_scheduling();
      }
      else
      {
        if (as_master)
        {
          as_master.get()->second.address = boost::none;
          request_registration_soon (as_master.get());
        }
      }

      break;
    }
    default:
    {
      LLOG ( ERROR, _logger
           , "Unhandled error (" << error.error_code() << ") :" << error.reason()
           );
    }
  }
}

namespace
{
  unsigned long total_memory_buffer_size (const we::type::activity_t& activity)
  {
    if (!activity.transition().module_call())
      return 0;

    expr::eval::context context;

    for ( std::pair< pnet::type::value::value_type, we::port_id_type>
            const& token_on_port
        : activity.input()
        )
    {
      context.bind_ref
        ( activity.transition().ports_input().at (token_on_port.second).name()
        , token_on_port.first
        );
    }

    return activity.transition().module_call()->memory_buffer_size_total (context);
  }
}

void GenericDaemon::submit ( const we::layer::id_type& job_id
                           , const we::type::activity_t& activity
                           )
try
{
  const we::type::schedule_data schedule_data
    (activity.transition().get_schedule_data<unsigned long> (activity.input(), "num_worker"));

  const double computational_cost (1.0); //!Note: use here an adequate cost provided by we! (can be the wall time)
  if (schedule_data.num_worker() && schedule_data.num_worker().get() == 0UL)
  {
    throw std::runtime_error ("invalid number of workers required: 0UL");
  }

  addJob ( job_id
         , activity.to_string()
         , boost::none
         , job_requirements_t ( activity.transition().requirements()
                              , schedule_data
                              , _virtual_memory_api->transfer_costs (activity)
                              , computational_cost
                              , total_memory_buffer_size (activity)
                              )
         );

  _scheduler.enqueueJob (job_id);
  request_scheduling();
}
catch (...)
{
  workflowEngine()->failed
    (job_id, fhg::util::current_exception_printer (": ").string());
}

void GenericDaemon::cancel (const we::layer::id_type& job_id)
{
  delay (std::bind (&GenericDaemon::delayed_cancel, this, job_id));
}
void GenericDaemon::delayed_cancel(const we::layer::id_type& job_id)
{
  boost::mutex::scoped_lock const _ (_scheduling_thread_mutex);

  Job* pJob (findJob (job_id));
  if (!pJob)
  {
    //! \note Job may have been removed between wfe requesting cancel
    //! and event thread handling this, which is not an error: wfe
    //! correctly handles that situation and expects us to ignore it.
    return;
  }

  pJob->CancelJob();

  const std::unordered_set<worker_id_t>
    workers_to_cancel (_worker_manager.workers_to_send_cancel (job_id));

  if (!workers_to_cancel.empty())
  {
    for (worker_id_t const& w : workers_to_cancel)
    {
      child_proxy ( this
                  , _worker_manager.address_by_worker (w).get()->second
                  ).cancel_job (job_id);
    }
  }
  else
  {
    workflowEngine()->canceled (job_id);
    pJob->CancelJobAck();
    _scheduler.delete_job (job_id);
    _scheduler.releaseReservation (job_id);
    deleteJob (job_id);
  }
}

void GenericDaemon::finished(const we::layer::id_type& id, const we::type::activity_t& result)
{
  Job* pJob = findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got finished message for old/unknown Job " + id);
  }

  pJob->JobFinished (result.to_string());

  if(!isSubscriber(pJob->owner()->_actual.get()->second.address.get()))
  {
    parent_proxy (this, pJob->owner()).job_finished (id, result.to_string());
  }

  if (m_guiService)
  {
    const sdpa::daemon::NotificationEvent evt
      ( {name()}
      , pJob->id()
      , NotificationEvent::STATE_FINISHED
      , result
      );

    m_guiService->notify (evt);
  }

  notify_subscribers<events::JobFinishedEvent> (id, id, result.to_string());
}

void GenericDaemon::failed( const we::layer::id_type& id
                          , std::string const & reason
                          )
{
  Job* pJob = findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got failed message for old/unknown Job " + id);
  }

  pJob->JobFailed (reason);

  if(!isSubscriber(pJob->owner()->_actual.get()->second.address.get()))
  {
    parent_proxy (this, pJob->owner()).job_failed (id, reason);
  }

  if (m_guiService)
  {
    const we::type::activity_t act (pJob->description());
    const sdpa::daemon::NotificationEvent evt
      ( {name()}
      , pJob->id()
      , NotificationEvent::STATE_FINISHED
      , act
      );

    m_guiService->notify (evt);
  }

  notify_subscribers<events::JobFailedEvent> (id, id, reason);
}

void GenericDaemon::canceled (const we::layer::id_type& job_id)
{
  Job* pJob (findJob (job_id));
  if (!pJob)
  {
    throw std::runtime_error ("rts_canceled (unknown job)");
  }

  pJob->CancelJobAck();

  //! \todo Should be if (job-has-owner), i.e. was-submitted-to-this-daemon
  if (!isTop())
  {
    parent_proxy (this, pJob->owner()).cancel_job_ack (job_id);

    deleteJob (job_id);
  }
}

    boost::optional<GenericDaemon::master_info_t::iterator>
      GenericDaemon::master_by_address (fhg::com::p2p::address_t const& address)
    {
      master_info_t::iterator it
        ( std::find_if ( _master_info.begin()
                       , _master_info.end()
                       , [&address] (master_info_t::value_type const& info)
                         {
                           return info.second.address == address;
                         }
                       )
        );
      return boost::make_optional (it != _master_info.end(), it);
    }

void GenericDaemon::handle_worker_registration_response
  ( fhg::com::p2p::address_t const& source
  , sdpa::events::worker_registration_response const* response
  )
{
  fhg::util::boost::get_or_throw<std::runtime_error>
    ( master_by_address (source)
    , "workerRegistrationAckEvent from source not in list of masters"
    );

  response->get();
}

void GenericDaemon::handleCapabilitiesGainedEvent
  (fhg::com::p2p::address_t const& source, const events::CapabilitiesGainedEvent* pCpbGainEvt)
{
	// tell the scheduler to add the capabilities of the worker source
  if (pCpbGainEvt->capabilities().empty())
  {
     return;
   }

 WorkerManager::worker_connections_t::right_map::iterator const worker
    ( fhg::util::boost::get_or_throw<std::runtime_error>
        (_worker_manager.worker_by_address (source), "capabilities_gained for unknown worker")
    );

  sdpa::capabilities_set_t workerCpbSet;

  for (const sdpa::capability_t& cpb : pCpbGainEvt->capabilities() )
  {
    // own capabilities have always the depth 0
    if( !isOwnCapability(cpb) )
    {
      sdpa::capability_t cpbMod(cpb);
      cpbMod.incDepth();
      workerCpbSet.insert(cpbMod);
    }
  }

  bool bModified (_worker_manager.add_worker_capabilities
    (worker->second, workerCpbSet));

  if(bModified)
  {
    _scheduler.reschedule_pending_jobs_matching_worker (worker->second);
    request_scheduling();
    if( !isTop() )
    {
      const sdpa::capabilities_set_t newWorkerCpbSet
        (_worker_manager.worker_capabilities (worker->second));

      if( !newWorkerCpbSet.empty() )
      {
        for (master_info_t::value_type const& info : _master_info)
        {
          if (info.second.address)
          {
            parent_proxy (this, *info.second.address).capabilities_gained
              (newWorkerCpbSet);
          }
        }
      }
    }
  }
}

void GenericDaemon::handleCapabilitiesLostEvent
  (fhg::com::p2p::address_t const& source, const events::CapabilitiesLostEvent* pCpbLostEvt)
{
  // tell the scheduler to remove the capabilities of the worker source

 WorkerManager::worker_connections_t::right_map::iterator const worker
    ( fhg::util::boost::get_or_throw<std::runtime_error>
        (_worker_manager.worker_by_address (source), "capabilities_lost for unknown worker")
    );

  if (_worker_manager.remove_worker_capabilities ( worker->second
                                                              , pCpbLostEvt->capabilities()
                                                              )
     )
  {
    for (master_info_t::value_type const& info : _master_info)
    {
      if (info.second.address)
      {
        parent_proxy (this, *info.second.address).capabilities_lost
          (pCpbLostEvt->capabilities());
      }
    }
  }
}

void GenericDaemon::handle_events()
{
  while (true)
  {
    const std::pair<fhg::com::p2p::address_t, events::SDPAEvent::Ptr> event
      (_event_queue.get());
    try
    {
      event.second->handleBy (event.first, this);
    }
    catch (...)
    {
      sendEventToOther<events::ErrorEvent>
        ( event.first
        , events::ErrorEvent::SDPA_EUNKNOWN
        , fhg::util::current_exception_printer (": ").string()
        );
    }
  }
}

void GenericDaemon::delay (std::function<void()> fun)
{
  _event_queue.put
    ( fhg::com::p2p::address_t()
    , events::SDPAEvent::Ptr (new events::delayed_function_call (fun))
    );
}

void GenericDaemon::request_registration_soon (master_info_t::iterator const& it)
{
  _registration_threads.start
    (std::bind (&GenericDaemon::do_registration_after_sleep, this, it));
}

void GenericDaemon::do_registration_after_sleep (master_info_t::iterator const& it)
{
  boost::this_thread::sleep (_registration_timeout);
  requestRegistration (it);
}

void GenericDaemon::requestRegistration (master_info_t::iterator const& it)
{
  try
  {
    it->second.address
      = _network_strategy.connect_to (it->second.host, it->second.port);
  }
  catch (std::exception const& ex)
  {
    request_registration_soon (it);
  }

  std::unique_lock<std::mutex> const guard_capabilites (mtx_cpb_);
  capabilities_set_t cpbSet (m_capabilities);

  _worker_manager.getCapabilities (cpbSet);

  parent_proxy (this, it).worker_registration (cpbSet);
}

void GenericDaemon::addCapability(const capability_t& cpb)
{
  std::unique_lock<std::mutex> const guard_capabilites (mtx_cpb_);
  m_capabilities.insert(cpb);
}

void GenericDaemon::unsubscribe(const fhg::com::p2p::address_t& id)
{
  std::unique_lock<std::mutex> const _ (mtx_subscriber_);
  _subscriptions.erase(id);
}

bool GenericDaemon::subscribedFor(const fhg::com::p2p::address_t& agId, const sdpa::job_id_t& jobId)
{
  return std::find
    (_subscriptions[agId].begin(), _subscriptions[agId].end(), jobId)
    != _subscriptions[agId].end();
}

void GenericDaemon::handleSubscribeEvent
  (fhg::com::p2p::address_t const& source, const events::SubscribeEvent* pEvt)
{
  const job_id_t& jobId (pEvt->job_id());

  std::unique_lock<std::mutex> const _ (mtx_subscriber_);

  Job* pJob (findJob (jobId));
  if (!pJob)
  {
    throw std::runtime_error ( "Could not subscribe for the job" + jobId
                             + ". The job does not exist!"
                             );
  }

  // allow to subscribe multiple times with different lists of job ids
  if (!subscribedFor (source, jobId))
  {
    _subscriptions[source].push_back (jobId);
  }

  sendEventToOther<events::SubscribeAckEvent> (source, jobId);

  // check if the subscribed jobs are already in a terminal state
  const sdpa::status::code status (pJob->getStatus());
  switch (status)
  {
  case sdpa::status::FINISHED:
    {
      sendEventToOther<events::JobFinishedEvent>
        (source, pJob->id(), pJob->result());
    }
    return;

  case sdpa::status::FAILED:
    {
      sendEventToOther<events::JobFailedEvent>
        (source, pJob->id(), pJob->error_message());
    }
    return;

  case sdpa::status::CANCELED:
    {
      sendEventToOther<events::CancelJobAckEvent> (source, pJob->id());
    }
    return;

  case sdpa::status::PENDING:
  case sdpa::status::RUNNING:
  case sdpa::status::CANCELING:
    // send nothing to the master if the job is not completed
    return;
  }

  INVALID_ENUM_VALUE (sdpa::status::code, status);
}

bool GenericDaemon::isSubscriber(const fhg::com::p2p::address_t& agentId)
{
  std::unique_lock<std::mutex> const _ (mtx_subscriber_);
  return _subscriptions.find (agentId) != _subscriptions.end();
}
    std::list<fhg::com::p2p::address_t> GenericDaemon::subscribers (job_id_t job_id) const
    {
      std::list<fhg::com::p2p::address_t> ret;

      for (subscriber_map_t::value_type const& subscription : _subscriptions)
      {
        for (job_id_t id : subscription.second)
        {
          if (id == job_id)
          {
            ret.push_back (subscription.first);
            break;
          }
        }
      }

      return ret;
    }

/**
 * Event SubmitJobAckEvent
 * Precondition: an acknowledgment event was received from a master
 * Action: - if the job was found, put the job into the state Running
 *         - move the job from the submitted queue of the worker worker_id, into its
 *           acknowledged queue
 *         - in the case when the worker was not found, trigger an exception and send back
 *           an error message
 * Postcondition: is either into the Running state or inexistent
 */
void GenericDaemon::handleSubmitJobAckEvent
  (fhg::com::p2p::address_t const& source, const events::SubmitJobAckEvent* pEvent)
{
  // Only, now should be state of the job updated to RUNNING
  // since it was not rejected, no error occurred etc ....
  //find the job ptrJob and call
  Job* ptrJob = findJob(pEvent->job_id());
  if(!ptrJob)
  {
    LLOG (ERROR, _logger,  "job " << pEvent->job_id()
                        << " could not be acknowledged:"
                        << " the job " <<  pEvent->job_id()
                        << " not found!"
                        );

    throw std::runtime_error
      ("Could not acknowledge job: " + pEvent->job_id() + " not found");
  }

  if(ptrJob->getStatus() == sdpa:: status::CANCELING)
    return;

  WorkerManager::worker_connections_t::right_map::iterator const worker
    ( fhg::util::boost::get_or_throw<std::runtime_error>
        ( _worker_manager.worker_by_address (source)
        , "submit_job_ack for unknown worker"
        )
    );

  ptrJob->Dispatch();
  _worker_manager.acknowledge_job_sent_to_worker
    (pEvent->job_id(), worker->second);
}

// respond to a worker that the JobFinishedEvent was received
void GenericDaemon::handleJobFinishedAckEvent
  (fhg::com::p2p::address_t const&, const events::JobFinishedAckEvent* pEvt)
{
  // The result was successfully delivered by the worker and the WE was notified
  // therefore, I can delete the job from the job map
  if (!findJob(pEvt->job_id()))
  {
    throw std::runtime_error ("Couldn't find the job!");
  }

  // delete it from the map when you receive a JobFinishedAckEvent!
  deleteJob(pEvt->job_id());
}

// respond to a worker that the JobFailedEvent was received
void GenericDaemon::handleJobFailedAckEvent
  (fhg::com::p2p::address_t const&, const events::JobFailedAckEvent* pEvt )
{
  if (!findJob(pEvt->job_id()))
  {
    throw std::runtime_error ("Couldn't find the job!");
  }

  // delete it from the map when you receive a JobFailedAckEvent!
  deleteJob(pEvt->job_id());
}

    void GenericDaemon::discover (we::layer::id_type discover_id, we::layer::id_type job_id)
    {
      delay ( std::bind ( &GenericDaemon::delayed_discover, this
                        , discover_id, job_id
                        )
            );
    }
    void GenericDaemon::delayed_discover
      (we::layer::id_type discover_id, we::layer::id_type job_id)
    {
      Job* pJob (findJob (job_id));

      std::unordered_set<worker_id_t> workers
        (_worker_manager.findSubmOrAckWorkers (job_id));

      if (pJob && !workers.empty())
      {
        for (worker_id_t const& w : workers)
        {
          child_proxy (this, _worker_manager.address_by_worker (w).get()->second)
              .discover_job_states (job_id, discover_id);
        }
      }
      else if (pJob)
      {
        workflowEngine()->discovered
          ( discover_id
          , discovery_info_t (job_id, pJob->getStatus(), discovery_info_set_t())
          );
      }
      else
      {
        workflowEngine()->discovered
          ( discover_id
          , discovery_info_t (job_id, boost::none, discovery_info_set_t())
          );
      }
    }

    void GenericDaemon::handleDiscoverJobStatesEvent
      (fhg::com::p2p::address_t const& source, const sdpa::events::DiscoverJobStatesEvent *pEvt)
    {
      Job* pJob (findJob (pEvt->job_id()));

      std::unordered_set<worker_id_t> workers
        (_worker_manager.findSubmOrAckWorkers (pEvt->job_id()));

      if (pJob && !workers.empty())
      {
        _discover_sources.emplace
          (std::make_pair (pEvt->discover_id(), pEvt->job_id()), source);

        for (worker_id_t const& w : workers)
        {
          child_proxy (this, _worker_manager.address_by_worker (w).get()->second)
            .discover_job_states (pEvt->job_id(), pEvt->discover_id());
        }
      }
      //! \todo Other criteria to know it was submitted to the
      //! wfe. All jobs are regarded as going to the wfe and the only
      //! way to prevent a loop is to check whether the discover comes
      //! out of the wfe. Special "worker" id?
      else if (pJob && workflowEngine())
      {
        _discover_sources.emplace
          (std::make_pair (pEvt->discover_id(), pEvt->job_id()), source);

        //! \todo There is a race here: between SubmitJobAck and
        //! we->submit(), there's still a lot of stuff. We can't
        //! guarantee, that the job is already submitted to the wfe!
        //! We need to handle the "pending" state.
        workflowEngine()->discover (pEvt->discover_id(), pEvt->job_id());
      }
      else if (pJob)
      {
        parent_proxy (this, source).discover_job_states_reply
          ( pEvt->discover_id()
          , discovery_info_t (pEvt->job_id(), pJob->getStatus(), discovery_info_set_t())
          );
      }
      else
      {
        parent_proxy (this, source).discover_job_states_reply
          ( pEvt->discover_id()
          , discovery_info_t (pEvt->job_id(), boost::none, discovery_info_set_t())
          );
      }
    }

    void GenericDaemon::discovered
      (we::layer::id_type discover_id, sdpa::discovery_info_t discover_result)
    {
      const std::pair<job_id_t, job_id_t> source_id
        (discover_id, discover_result.job_id());

      parent_proxy (this, _discover_sources.at (source_id))
        .discover_job_states_reply (discover_id, discover_result);

      _discover_sources.erase (source_id);
    }

    void GenericDaemon::handleDiscoverJobStatesReplyEvent
      (fhg::com::p2p::address_t const&, const sdpa::events::DiscoverJobStatesReplyEvent* e)
    {
      const std::pair<job_id_t, job_id_t> source_id
        (e->discover_id(), e->discover_result().job_id());
      const std::unordered_map<std::pair<job_id_t, job_id_t>, fhg::com::p2p::address_t>::iterator
        source (_discover_sources.find (source_id));

      if (source == _discover_sources.end())
      {
        workflowEngine()->discovered (e->discover_id(), e->discover_result());
      }
      else
      {
        parent_proxy (this, source->second).discover_job_states_reply
          (e->discover_id(), e->discover_result());

        _discover_sources.erase (source);
      }
    }

    void GenericDaemon::handleBacklogNoLongerFullEvent
      (fhg::com::p2p::address_t const& source, const events::BacklogNoLongerFullEvent*)
    {
      boost::optional<WorkerManager::worker_connections_t::right_map::iterator> const as_worker
          (_worker_manager.worker_by_address (source));

      _worker_manager.set_worker_backlog_full (as_worker.get()->second, false);
      request_scheduling ();
    }

    namespace
    {
      template<typename Map>
        typename Map::mapped_type take (Map& map, typename Map::key_type key)
      {
        typename Map::iterator const it (map.find (key));
        if (it == map.end())
        {
          throw std::runtime_error ("take: key " + key + " not found");
        }

        typename Map::mapped_type v (std::move (it->second));
        map.erase (it);
        return v;
      }
    }

    void GenericDaemon::handle_put_token
      (fhg::com::p2p::address_t const& source, const events::put_token* event)
    try
    {
      Job* job (findJob (event->job_id()));

      std::unordered_set<worker_id_t> workers
        (_worker_manager.findSubmOrAckWorkers (event->job_id()));

      if (!job || (workers.empty() && !workflowEngine()))
      {
        throw std::runtime_error
          ("unable to put token: " + event->job_id() + " unknown or not running");
      }

      if (!workers.empty())
      {
        _put_token_source.emplace (event->put_token_id(), source);

        for (worker_id_t const& w : workers)
        {
          child_proxy (this, _worker_manager.address_by_worker (w).get()->second)
            .put_token ( event->job_id()
                       , event->put_token_id()
                       , event->place_name()
                       , event->value()
                       );
        }
      }
      //! \todo Other criteria to know it was submitted to the
      //! wfe. All jobs are regarded as going to the wfe and the only
      //! way to prevent a loop is to check whether the put_token
      //! comes out of the wfe. Special "worker" id?
      else
      {
        _put_token_source.emplace (event->put_token_id(), source);

        //! \todo There is a race here: between SubmitJobAck and
        //! we->submit(), there's still a lot of stuff. We can't
        //! guarantee, that the job is already submitted to the wfe!
        //! We need to handle the "pending" state.
        workflowEngine()->put_token ( event->job_id()
                                    , event->put_token_id()
                                    , event->place_name()
                                    , event->value()
                                    );
      }
    }
    catch (...)
    {
      parent_proxy (this, source).put_token_response
        (event->put_token_id(), std::current_exception());
    }
    void GenericDaemon::handle_put_token_response
      ( fhg::com::p2p::address_t const&
      , events::put_token_response const* event
      )
    {
      parent_proxy (this, take (_put_token_source, event->put_token_id()))
        .put_token_response (event->put_token_id(), event->exception());
    }
    void GenericDaemon::token_put
      ( std::string put_token_id
      , boost::optional<std::exception_ptr> error
      )
    {
      parent_proxy (this, take (_put_token_source, put_token_id))
        .put_token_response (put_token_id, error);
    }

    void GenericDaemon::handle_workflow_response
      (fhg::com::p2p::address_t const& source, const events::workflow_response* event)
    try
    {
      Job* job (findJob (event->job_id()));

      std::unordered_set<worker_id_t> workers
        (_worker_manager.findSubmOrAckWorkers (event->job_id()));

      if (!job || (workers.empty() && !workflowEngine()))
      {
        throw std::runtime_error
          ( "unable to request workflow response: " + event->job_id()
          + " unknown or not running"
          );
      }

      if (!workers.empty())
      {
        _workflow_response_source.emplace (event->workflow_response_id(), source);

        for (worker_id_t const& w : workers)
        {
          child_proxy (this, _worker_manager.address_by_worker (w).get()->second)
            .workflow_response ( event->job_id()
                               , event->workflow_response_id()
                               , event->place_name()
                               , event->value()
                               );
        }
      }
      //! \todo Other criteria to know it was submitted to the
      //! wfe. All jobs are regarded as going to the wfe and the only
      //! way to prevent a loop is to check whether the
      //! workflow_response comes out of the wfe. Special "worker" id?
      else
      {
        _workflow_response_source.emplace (event->workflow_response_id(), source);

        //! \todo There is a race here: between SubmitJobAck and
        //! we->submit(), there's still a lot of stuff. We can't
        //! guarantee, that the job is already submitted to the wfe!
        //! We need to handle the "pending" state.
        workflowEngine()->request_workflow_response ( event->job_id()
                                                    , event->workflow_response_id()
                                                    , event->place_name()
                                                    , event->value()
                                                    );
      }
    }
    catch (...)
    {
      parent_proxy (this, source).workflow_response_response
        (event->workflow_response_id(), std::current_exception());
    }
    void GenericDaemon::handle_workflow_response_response
      ( fhg::com::p2p::address_t const&
      , events::workflow_response_response const* event
      )
    {
      parent_proxy (this, take (_workflow_response_source, event->workflow_response_id()))
        .workflow_response_response (event->workflow_response_id(), event->content());
    }
    void GenericDaemon::workflow_response_response
      ( std::string workflow_response_id
      , boost::variant<std::exception_ptr, pnet::type::value::value_type> result
      )
    {
      parent_proxy (this, take (_workflow_response_source, workflow_response_id))
        .workflow_response_response (workflow_response_id, result);
    }


void GenericDaemon::handleRetrieveJobResultsEvent
  (fhg::com::p2p::address_t const& source, const events::RetrieveJobResultsEvent* pEvt )
{
  Job* pJob = findJob(pEvt->job_id());
  if (!pJob)
  {
    throw std::runtime_error ("Inexistent job: "+pEvt->job_id());
  }

  if (!sdpa::status::is_terminal (pJob->getStatus()))
  {
    throw std::runtime_error
      ( "Not allowed to request results for a non-terminated job, its current status is : "
      +  sdpa::status::show(pJob->getStatus())
      );
  }

  parent_proxy (this, source).retrieve_job_results_reply
    (pEvt->job_id(), pJob->result());
}

void GenericDaemon::handleQueryJobStatusEvent
  (fhg::com::p2p::address_t const& source, const events::QueryJobStatusEvent* pEvt )
{
  sdpa::job_id_t jobId = pEvt->job_id();

  Job* pJob (findJob(jobId));
  if (!pJob)
  {
    throw std::runtime_error ("Inexistent job: "+pEvt->job_id());
  }

  parent_proxy (this, source).query_job_status_reply
    (pJob->id(), pJob->getStatus(), pJob->error_message());
}

}}

namespace sdpa
{
  namespace daemon
  {
    void GenericDaemon::scheduling_thread()
    {
      for (;;)
      {
        boost::mutex::scoped_lock lock (_scheduling_thread_mutex);
        _scheduling_thread_notifier.wait (lock);

        std::list<worker_id_t> new_workers
          (_new_workers_added.get_and_clear());

        for (worker_id_t const& w : new_workers)
        {
          _scheduler.reschedule_pending_jobs_matching_worker (w);
        }

        _scheduler.assignJobsToWorkers();
        _scheduler.start_pending_jobs
          (std::bind (&GenericDaemon::serveJob, this, std::placeholders::_1, std::placeholders::_2));
      }
    }

    void GenericDaemon::request_scheduling()
    {
      boost::mutex::scoped_lock const _ (_scheduling_thread_mutex);
      _scheduling_thread_notifier.notify_one();
    }

    void GenericDaemon::request_rescheduling (worker_id_t const& w)
    {
      _new_workers_added.push (w);
      _scheduling_thread_notifier.notify_one();
    }

    GenericDaemon::child_proxy::child_proxy
        (GenericDaemon* that, fhg::com::p2p::address_t const& address)
      : _that (that)
      , _address (address)
    {}

    void GenericDaemon::child_proxy::worker_registration_response
      (boost::optional<std::exception_ptr> error) const
    {
      _that->sendEventToOther<events::worker_registration_response>
        (_address, std::move (error));
    }

    void GenericDaemon::child_proxy::submit_job ( boost::optional<job_id_t> id
                                                , job_desc_t description
                                                , std::set<worker_id_t> const& workers
                                                ) const
    {
      _that->sendEventToOther<events::SubmitJobEvent>
        (_address, id, description, workers);
    }

    void GenericDaemon::child_proxy::cancel_job (job_id_t id) const
    {
      _that->sendEventToOther<events::CancelJobEvent> (_address, id);
    }

    void GenericDaemon::child_proxy::job_failed_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::JobFailedAckEvent> (_address, id);
    }

    void GenericDaemon::child_proxy::job_finished_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::JobFinishedAckEvent> (_address, id);
    }

    void GenericDaemon::child_proxy::discover_job_states
      (job_id_t job_id, job_id_t discover_id) const
    {
      _that->sendEventToOther<events::DiscoverJobStatesEvent>
        (_address, job_id, discover_id);
    }

    void GenericDaemon::child_proxy::put_token
      ( job_id_t job_id
      , std::string put_token_id
      , std::string place_name
      , pnet::type::value::value_type value
      ) const
    {
      _that->sendEventToOther<events::put_token>
        (_address, job_id, put_token_id, place_name, value);
    }

    void GenericDaemon::child_proxy::workflow_response
      ( job_id_t job_id
      , std::string workflow_response_id
      , std::string place_name
      , pnet::type::value::value_type value
      ) const
    {
      _that->sendEventToOther<events::workflow_response>
        (_address, job_id, workflow_response_id, place_name, value);
    }

    GenericDaemon::parent_proxy::parent_proxy
        (GenericDaemon* that, fhg::com::p2p::address_t const& address)
      : _that (that)
      , _address (address)
    {}
    GenericDaemon::parent_proxy::parent_proxy
        (GenericDaemon* that, master_info_t::iterator const& master)
      : parent_proxy (that, master->second.address.get())
    {}
    GenericDaemon::parent_proxy::parent_proxy
        (GenericDaemon* that, opaque_job_master_t const& job_master)
      : parent_proxy (that, job_master->_actual.get())
    {}

    void GenericDaemon::parent_proxy::worker_registration
      ( capabilities_set_t capabilities
      ) const
    {
      _that->sendEventToOther<events::WorkerRegistrationEvent>
        ( _address
        , _that->name(), capabilities, 0, true, fhg::util::hostname()
        );
    }

    void GenericDaemon::parent_proxy::notify_shutdown() const
    {
      _that->sendEventToOther<events::ErrorEvent>
        (_address, events::ErrorEvent::SDPA_ENODE_SHUTDOWN, "");
    }

    void GenericDaemon::parent_proxy::job_failed
      (job_id_t id, std::string message) const
    {
      _that->sendEventToOther<events::JobFailedEvent> (_address, id, message);
    }

    void GenericDaemon::parent_proxy::job_finished
      (job_id_t id, job_result_t result) const
    {
      _that->sendEventToOther<events::JobFinishedEvent> (_address, id, result);
    }

    void GenericDaemon::parent_proxy::cancel_job_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::CancelJobAckEvent> (_address, id);
    }

    void GenericDaemon::parent_proxy::delete_job_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::DeleteJobAckEvent> (_address, id);
    }

    void GenericDaemon::parent_proxy::submit_job_ack (job_id_t id) const
    {
      _that->sendEventToOther<events::SubmitJobAckEvent> (_address, id);
    }

    void GenericDaemon::parent_proxy::capabilities_gained
      (capabilities_set_t capabilities) const
    {
      _that->sendEventToOther<events::CapabilitiesGainedEvent>
        (_address, capabilities);
    }

    void GenericDaemon::parent_proxy::capabilities_lost
      (capabilities_set_t capabilities) const
    {
      _that->sendEventToOther<events::CapabilitiesLostEvent>
        (_address, capabilities);
    }

    void GenericDaemon::parent_proxy::discover_job_states_reply
      (job_id_t discover_id, discovery_info_t info) const
    {
      _that->sendEventToOther<events::DiscoverJobStatesReplyEvent>
        (_address, discover_id, info);
    }

    void GenericDaemon::parent_proxy::query_job_status_reply
      (job_id_t id, status::code status, std::string error_message) const
    {
      _that->sendEventToOther<events::JobStatusReplyEvent>
        (_address, id, status, error_message);
    }

    void GenericDaemon::parent_proxy::retrieve_job_results_reply
      (job_id_t id, job_result_t result) const
    {
      _that->sendEventToOther<events::JobResultsReplyEvent>
        (_address, id, result);
    }

    void GenericDaemon::parent_proxy::put_token_response
      (std::string put_token_id, boost::optional<std::exception_ptr> error) const
    {
      _that->sendEventToOther<events::put_token_response>
        (_address, put_token_id, error);
    }

    void GenericDaemon::parent_proxy::workflow_response_response
      ( std::string workflow_response_id
      , boost::variant<std::exception_ptr, pnet::type::value::value_type> content
      ) const
    {
      _that->sendEventToOther<events::workflow_response_response>
        (_address, workflow_response_id, content);
    }
  }
}
