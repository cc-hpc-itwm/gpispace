// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/context.hpp>
#include <drts/private/scoped_allocation.hpp>

#include <fhgcom/message.hpp>
#include <fhgcom/peer.hpp>

#include <fhg/util/thread/bounded_queue.hpp>
#include <fhg/util/thread/queue.hpp>
#include <fhg/util/thread/set.hpp>

#include <gpi-space/pc/client/api.hpp>

#include <sdpa/capability.hpp>
#include <sdpa/daemon/NotificationEvent.hpp>
#include <sdpa/daemon/NotificationService.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

#include <we/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/activity.hpp>

#include <hwloc.h>

#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <atomic>
#include <functional>
#include <list>
#include <map>
#include <mutex>
#include <string>
#include <unordered_set>

struct wfe_task_t
{
  enum state_t
  {
    PENDING
  , CANCELED
  , FINISHED
  , FAILED
  };

  std::string id;
  state_t state;
  we::type::activity_t activity;
  drts::worker::context context;

  wfe_task_t ( std::string id
             , std::string const& description
             , std::string worker_name
             , std::list<std::string> workers
             )
    : id (id)
    , state (PENDING)
    , activity (description)
    , context (worker_name, workers)
  {}
};

class numa_socket_setter
{
public:
  numa_socket_setter (size_t target_socket);
  ~numa_socket_setter();

private:
  hwloc_topology_t m_topology;
};

class DRTSImpl : public sdpa::events::EventHandler
{
  typedef std::map<std::string, fhg::com::p2p::address_t> map_of_masters_t;

public:
  class Job
  {
  public:
    enum state_t
    {
      PENDING
    , RUNNING
    , FINISHED
    , FAILED
    , CANCELED
    };

    using owner_type = map_of_masters_t::const_iterator;

    Job ( std::string const& jobid
        , std::string const& description
        , owner_type const& owner
        , std::list<std::string> const& worker_list
        )
      : id (jobid)
      , description (description)
      , owner (owner)
      , worker_list (worker_list)
      , state (Job::PENDING)
      , result()
      , message ("")
    {}

    std::string const id;
    std::string const description;
    owner_type const owner;
    std::list<std::string> const worker_list;
    std::atomic<state_t> state;
    std::string result;
    std::string message;
  };

private:
  typedef std::map<std::string, std::shared_ptr<DRTSImpl::Job>> map_of_jobs_t;

public:
  DRTSImpl
    ( std::function<void()> request_stop
    , boost::asio::io_service& peer_io_service
    , boost::optional<std::pair<std::string, boost::asio::io_service&>> gui_info
    , std::map<std::string, std::string> config_variables
    , std::string const& kernel_name
    , gpi::pc::client::api_t /*const*/* virtual_memory_socket
    , gspc::scoped_allocation /*const*/* shared_memory
    );
  ~DRTSImpl();

  virtual void handleWorkerRegistrationAckEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::WorkerRegistrationAckEvent *e) override;
  virtual void handleSubmitJobEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::SubmitJobEvent *e) override;
  virtual void handleCancelJobEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent *e) override;
  virtual void handleJobFailedAckEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::JobFailedAckEvent *e) override;
  virtual void handleJobFinishedAckEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::JobFinishedAckEvent *e) override;
  virtual void handleDiscoverJobStatesEvent
    (fhg::com::p2p::address_t const& source, const sdpa::events::DiscoverJobStatesEvent*) override;

private:
  void event_thread();
  void job_execution_thread();

  void resend_outstanding_events (map_of_masters_t::const_iterator const&);

  void send_job_result_to_master (std::shared_ptr<DRTSImpl::Job> const& job);

  void start_receiver();

  template<typename Event, typename... Args>
    void send_event (fhg::com::p2p::address_t const& destination, Args&&... args);

  fhg::log::Logger::ptr_t _logger;

  std::function<void()> _request_stop;

  bool m_shutting_down;

  std::string m_my_name;

  boost::optional<numa_socket_setter> _numa_socket_setter;

  mutable std::mutex _currently_executed_tasks_mutex;
  std::map<std::string, wfe_task_t *> _currently_executed_tasks;

  we::loader::loader m_loader;

  boost::optional<sdpa::daemon::NotificationService> _notification_service;

  gpi::pc::client::api_t /*const*/* _virtual_memory_api;
  gspc::scoped_allocation /*const*/* _shared_memory;

  struct mark_remaining_tasks_as_canceled_helper
  {
    ~mark_remaining_tasks_as_canceled_helper();

    std::mutex& _currently_executed_tasks_mutex;
    std::map<std::string, wfe_task_t *>& _currently_executed_tasks;
  } _mark_remaining_tasks_as_canceled_helper
    = {_currently_executed_tasks_mutex, _currently_executed_tasks};

  std::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>
    m_peer_thread;
  std::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  //! \todo Two sets for connected and unconnected masters?
  map_of_masters_t m_masters;

  fhg::thread::queue<std::pair<fhg::com::p2p::address_t, sdpa::events::SDPAEvent::Ptr>>
    m_event_queue;

  mutable std::mutex m_job_map_mutex;

  map_of_jobs_t m_jobs;

  fhg::thread::bounded_queue<std::shared_ptr<DRTSImpl::Job>> m_pending_jobs;

  mutable std::mutex _guard_backlogfull_notified_masters;
  std::unordered_set<fhg::com::p2p::address_t> _masters_backlogfull_notified;

  struct peer_stopper
  {
    ~peer_stopper();
    std::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>&
      m_peer_thread;
    std::shared_ptr<fhg::com::peer_t>& m_peer;
  } _peer_stopper = {m_peer_thread, m_peer};

  boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    m_event_thread;
  std::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>
    m_execution_thread;
};
