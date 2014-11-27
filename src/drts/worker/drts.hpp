// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/context.hpp>
#include <drts/worker/job.hpp>

#include <fhgcom/message.hpp>
#include <fhgcom/peer.hpp>

#include <fhg/util/thread/queue.hpp>
#include <fhg/util/thread/set.hpp>

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
  std::string error_message;

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

class WFEImpl
{
public:
  WFEImpl ( fhg::log::Logger::ptr_t
          , boost::optional<std::size_t> target_socket
          , std::string search_path
          , boost::optional<std::pair<std::string, boost::asio::io_service&>> gui_info
          , std::string worker_name
          );
  ~WFEImpl();

private:
  void emit_task (const wfe_task_t& task);

public:
  int execute ( std::string const &job_id
              , std::string const &job_description
              , we::type::activity_t & result
              , std::string & error_message
              , std::list<std::string> const & worker_list
              );

  void cancel (std::string const &job_id);

private:
  fhg::log::Logger::ptr_t _logger;

  boost::optional<numa_socket_setter> _numa_socket_setter;

  std::string _worker_name;

  mutable boost::mutex _currently_executed_tasks_mutex;
  std::map<std::string, wfe_task_t *> _currently_executed_tasks;

  we::loader::loader m_loader;

  boost::optional<sdpa::daemon::NotificationService> _notification_service;
};

class DRTSImpl : public sdpa::events::EventHandler
{
  typedef std::map<std::string, boost::optional<fhg::com::p2p::address_t>>
    map_of_masters_t;

  typedef std::map< std::string
                  , boost::shared_ptr<drts::Job>
                  > map_of_jobs_t;
public:
  DRTSImpl
    ( std::function<void()> request_stop
    , boost::asio::io_service& peer_io_service
    , boost::asio::io_service& kvs_client_io_service
    , boost::optional<std::pair<std::string, boost::asio::io_service&>> gui_info
    , std::map<std::string, std::string> config_variables
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

  void send_job_result_to_master (boost::shared_ptr<drts::Job> const & job);

  void request_registration_soon();
  void request_registration_after_sleep();

  void start_connect ();

  void start_receiver();
  void handle_recv ( boost::system::error_code const & ec
                   , boost::optional<fhg::com::p2p::address_t> source_name
                   );

  void send_event (fhg::com::p2p::address_t const& destination, sdpa::events::SDPAEvent *e);
  void send_event (fhg::com::p2p::address_t const& destination, sdpa::events::SDPAEvent::Ptr const & evt);

  void dispatch_event
    (fhg::com::p2p::address_t const& source, sdpa::events::SDPAEvent::Ptr const &evt);

  fhg::log::Logger::ptr_t _logger;

  std::function<void()> _request_stop;

  fhg::com::kvs::kvsc_ptr_t _kvs_client;

  bool m_shutting_down;

  std::string m_my_name;

  WFEImpl m_wfe;

  boost::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>
    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  //! \todo Two sets for connected and unconnected masters?
  map_of_masters_t m_masters;
  std::size_t m_max_reconnect_attempts;
  std::size_t m_reconnect_counter;

  fhg::thread::queue<std::pair<fhg::com::p2p::address_t, sdpa::events::SDPAEvent::Ptr>>
    m_event_queue;
  boost::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>
    m_event_thread;
  boost::shared_ptr<boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>>
    m_execution_thread;

  mutable boost::mutex m_job_map_mutex;
  mutable boost::mutex m_job_computed_mutex;
  boost::condition_variable     m_job_computed;
  mutable boost::mutex m_job_arrived_mutex;
  mutable boost::mutex m_reconnect_counter_mutex;
  boost::condition_variable     m_job_arrived;

  mutable boost::mutex m_capabilities_mutex;
  std::set<sdpa::Capability> m_virtual_capabilities;

  // jobs + their states
  size_t m_backlog_size;
  map_of_jobs_t m_jobs;

  fhg::thread::queue<boost::shared_ptr<drts::Job>> m_pending_jobs;

  fhg::thread::set _registration_threads;
};
