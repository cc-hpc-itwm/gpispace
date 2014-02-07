// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/job.hpp>

#include <fhgcom/message.hpp>
#include <fhgcom/peer.hpp>

#include <fhg/util/keep_alive.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhg/util/thread/queue.hpp>
#include <fhg/util/thread/set.hpp>

#include <gspc/drts/context.hpp>
#include <gspc/net/frame.hpp>
#include <gspc/net/io.hpp>
#include <gspc/net/server.hpp>
#include <gspc/net/server/queue_manager.hpp>
#include <gspc/net/server/service_demux.hpp>
#include <gspc/net/user.hpp>

#include <sdpa/capability.hpp>
#include <sdpa/daemon/NotificationEvent.hpp>
#include <sdpa/daemon/NotificationService.hpp>
#include <sdpa/events/SDPAEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

#include <we/context.hpp>
#include <we/loader/loader.hpp>
#include <we/type/activity.hpp>

#include <hwloc.h>

#include <boost/function.hpp>
#include <boost/optional.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>

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
  int        errc;
  we::type::activity_t activity;
  gspc::drts::context context;
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
  WFEImpl ( boost::optional<std::size_t> target_socket
          , std::string search_path
          , boost::optional<std::string> gui_url
          , std::string worker_name
          , gspc::net::server::service_demux_t& service_demux
          );
  ~WFEImpl();

private:
  void emit_task ( const wfe_task_t& task
                 , sdpa::daemon::NotificationEvent::state_t state
                 );

public:
  int execute ( std::string const &job_id
              , std::string const &job_description
              , we::type::activity_t & result
              , std::string & error_message
              , std::list<std::string> const & worker_list
              );

  int cancel (std::string const &job_id);

private:
  void service_current_job ( std::string const &
                           , gspc::net::frame const &rqst
                           , gspc::net::user_ptr user
                           );

  void service_get_search_path ( std::string const &
                               , gspc::net::frame const &rqst
                               , gspc::net::user_ptr user
                               );

  void service_set_search_path ( std::string const &
                               , gspc::net::frame const &rqst
                               , gspc::net::user_ptr user
                               );

  boost::optional<numa_socket_setter> _numa_socket_setter;

  std::string _worker_name;

  mutable boost::mutex m_mutex;
  std::map<std::string, wfe_task_t *> m_task_map;
  fhg::thread::queue<wfe_task_t*> m_tasks;

  mutable boost::mutex m_current_task_mutex;
  wfe_task_t *m_current_task;

  we::loader::loader m_loader;

  boost::optional<sdpa::daemon::NotificationService> _notification_service;

  gspc::net::server::scoped_service_handler _current_job_service;
  gspc::net::server::scoped_service_handler _set_search_path_service;
  gspc::net::server::scoped_service_handler _get_search_path_service;
};

class DRTSImpl : public sdpa::events::EventHandler
{
  typedef std::map<std::string, bool> map_of_masters_t;

  typedef std::map< std::string
                  , boost::shared_ptr<drts::Job>
                  > map_of_jobs_t;
  typedef std::map<std::string, sdpa::Capability> map_of_capabilities_t;
public:
  DRTSImpl (boost::function<void()> request_stop, std::map<std::string, std::string> config_variables);
  ~DRTSImpl();

  virtual void handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent *e);
  virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent *e);
  virtual void handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent*);
  virtual void handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent*);
  virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent *);
  virtual void handleErrorEvent(const sdpa::events::ErrorEvent *);
  virtual void handleQueryJobStatusEvent(const sdpa::events::QueryJobStatusEvent *);
  virtual void handleRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent *);
  virtual void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent *e);
  virtual void handleCancelJobEvent(const sdpa::events::CancelJobEvent *e);
  virtual void handleJobFailedAckEvent(const sdpa::events::JobFailedAckEvent *e);
  virtual void handleJobFinishedAckEvent(const sdpa::events::JobFinishedAckEvent *e);
  virtual void handleCancelJobAckEvent(const sdpa::events::CancelJobAckEvent *);
  virtual void handleJobFailedEvent(const sdpa::events::JobFailedEvent *);
  virtual void handleJobFinishedEvent(const sdpa::events::JobFinishedEvent *);
  virtual void handleJobResultsReplyEvent(const sdpa::events::JobResultsReplyEvent *);
  virtual void handleJobStatusReplyEvent(const sdpa::events::JobStatusReplyEvent *);
  virtual void handleSubmitJobAckEvent(const sdpa::events::SubmitJobAckEvent *);

private:
  // threads
  void event_thread ();
  void job_execution_thread ();

  void add_virtual_capability (std::string const &cap);
  void del_virtual_capability (std::string const &cap);

  void service_capability_add ( std::string const &
                              , gspc::net::frame const &rqst
                              , gspc::net::user_ptr user
                              );
  void service_capability_del ( std::string const &
                              , gspc::net::frame const &rqst
                              , gspc::net::user_ptr user
                              );
  void service_capability_get ( std::string const &
                              , gspc::net::frame const &rqst
                              , gspc::net::user_ptr user
                              );

  void notify_capabilities_to_master (std::string const &master);
  void notify_capability_gained (sdpa::Capability const &cap);
  void notify_capability_lost (sdpa::Capability const &cap);

  void resend_outstanding_events (std::string const &master);

  void send_job_result_to_master (boost::shared_ptr<drts::Job> const & job);

  void request_registration_soon();
  void request_registration_after_sleep();

  void start_connect ();

  void start_receiver();
  void handle_recv (boost::system::error_code const & ec);

  void send_event (sdpa::events::SDPAEvent *e);
  void send_event (sdpa::events::SDPAEvent::Ptr const & evt);

  void dispatch_event (sdpa::events::SDPAEvent::Ptr const &evt);

  gspc::net::initializer _net_initializer;
  gspc::net::server::service_demux_t& _service_demux;
  gspc::net::server::queue_manager_t _queue_manager;

  boost::function<void()> _request_stop;

  //! \todo don't be pointer!
  fhg::util::keep_alive* _kvs_keep_alive;

  bool m_shutting_down;

  WFEImpl *m_wfe;

  boost::shared_ptr<boost::thread>    m_peer_thread;
  boost::shared_ptr<fhg::com::peer_t> m_peer;
  fhg::com::message_t m_message;
  std::string m_my_name;
  //! \todo Two sets for connected and unconnected masters?
  map_of_masters_t m_masters;
  std::size_t m_max_reconnect_attempts;
  std::size_t m_reconnect_counter;

  fhg::thread::queue<sdpa::events::SDPAEvent::Ptr>  m_event_queue;
  boost::shared_ptr<boost::thread>    m_event_thread;
  boost::shared_ptr<boost::thread>    m_execution_thread;

  mutable boost::mutex m_job_map_mutex;
  mutable boost::mutex m_job_computed_mutex;
  boost::condition_variable     m_job_computed;
  mutable boost::mutex m_job_arrived_mutex;
  mutable boost::mutex m_reconnect_counter_mutex;
  boost::condition_variable     m_job_arrived;

  fhg::util::thread::event<std::string> m_connected_event;

  mutable boost::mutex m_capabilities_mutex;
  map_of_capabilities_t m_virtual_capabilities;

  // jobs + their states
  size_t m_backlog_size;
  map_of_jobs_t m_jobs;

  fhg::thread::queue<boost::shared_ptr<drts::Job> > m_pending_jobs;

  gspc::net::server_ptr_t m_server;

  fhg::thread::set _registration_threads;
};
