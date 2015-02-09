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

#include <functional>
#include <list>
#include <map>
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

  wfe_task_t (std::string id, std::string worker_name, std::list<std::string> workers);
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
    typedef boost::mutex mutex_type;
    typedef boost::condition_variable condition_type;
    typedef boost::unique_lock<mutex_type> lock_type;
  public:
    enum state_t
      {
        PENDING = 0
      , RUNNING
      , FINISHED
      , FAILED
      , CANCELED
      };

    struct ID
    {
      explicit ID(std::string const &s)
        : value(s)
      {}

      const std::string value;
    };

    struct Description
    {
      explicit Description(std::string const &s)
        : value(s)
      {}

      const std::string value;
    };

    using owner_type = map_of_masters_t::const_iterator;

    explicit
    Job( Job::ID const &jobid
       , Job::Description const &description
       , owner_type const&
       , std::list<std::string> const& worker_list
       );

    inline state_t state () const { lock_type lck(m_mutex); return m_state; }
    state_t cmp_and_swp_state( state_t expected
                             , state_t newstate
                             );
    inline Job& set_state (state_t s) {
      lock_type    lck(m_mutex); m_state = s;
      return *this;
    }

    std::string const & id() const { return m_id; }
    std::string const & description() const { return m_input_description; }
    owner_type const& owner() const { return m_owner; }

    std::string const & result() const {
      lock_type lck(m_mutex); return m_result;
    }
    Job & set_result(std::string const &r) {
      lock_type lck(m_mutex); m_result = r;
      return *this;
    }

    std::string const & message() const {
      lock_type lck(m_mutex); return m_message;
    }

    Job& set_message (std::string const &s) {
      lock_type lck(m_mutex); m_message = s;
      return *this;
    }

    std::list<std::string> const &worker_list () const
    {
      return m_worker_list;
    }

  private:
    inline void    state (state_t s) { lock_type lck(m_mutex); m_state = s; }
    mutable mutex_type m_mutex;

    std::string m_id;
    std::string m_input_description;
    owner_type m_owner;
    state_t     m_state;
    std::string m_result;
    std::string m_message;
    std::list<std::string> m_worker_list;
  };

private:
  typedef std::map< std::string
                  , boost::shared_ptr<DRTSImpl::Job>
                  > map_of_jobs_t;
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
  // threads
  void event_thread ();
  void job_execution_thread ();

  void resend_outstanding_events (map_of_masters_t::const_iterator const&);

  void send_job_result_to_master (boost::shared_ptr<DRTSImpl::Job> const & job);

  void start_receiver();

  template<typename Event, typename... Args>
    void send_event ( fhg::com::p2p::address_t const& destination
                    , Args&&... args
                    );

  fhg::log::Logger::ptr_t _logger;

  std::function<void()> _request_stop;

  bool m_shutting_down;

  std::string m_my_name;

  boost::optional<numa_socket_setter> _numa_socket_setter;

  mutable boost::mutex _currently_executed_tasks_mutex;
  std::map<std::string, wfe_task_t *> _currently_executed_tasks;

  we::loader::loader m_loader;

  boost::optional<sdpa::daemon::NotificationService> _notification_service;

  gpi::pc::client::api_t /*const*/* _virtual_memory_api;
  gspc::scoped_allocation /*const*/* _shared_memory;

  struct mark_remaining_tasks_as_canceled_helper
  {
    ~mark_remaining_tasks_as_canceled_helper();

    boost::mutex& _currently_executed_tasks_mutex;
    std::map<std::string, wfe_task_t *>& _currently_executed_tasks;
  } _mark_remaining_tasks_as_canceled_helper
    = {_currently_executed_tasks_mutex, _currently_executed_tasks};

  boost::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>
    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  //! \todo Two sets for connected and unconnected masters?
  map_of_masters_t m_masters;

  fhg::thread::queue<std::pair<fhg::com::p2p::address_t, sdpa::events::SDPAEvent::Ptr>>
    m_event_queue;

  mutable boost::mutex m_job_map_mutex;

  // jobs + their states
  map_of_jobs_t m_jobs;

  fhg::thread::bounded_queue<boost::shared_ptr<DRTSImpl::Job>> m_pending_jobs;

  mutable boost::mutex _guard_backlogfull_notified_masters;
  std::unordered_set<fhg::com::p2p::address_t> _masters_backlogfull_notified;

  struct peer_stopper
  {
    ~peer_stopper();
    boost::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>&
      m_peer_thread;
    boost::shared_ptr<fhg::com::peer_t>& m_peer;
  } _peer_stopper = {m_peer_thread, m_peer};

  boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
    m_event_thread;
  boost::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>
    m_execution_thread;
};
