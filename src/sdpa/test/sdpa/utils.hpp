#pragma once

#include <sdpa/client.hpp>
#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

#include <fhgcom/peer.hpp>

#include <logging/stdout_sink.hpp>

#include <fhg/util/thread/event.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <we/type/activity.hpp>

#include <boost/optional.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <condition_variable>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>

namespace boost
{
  namespace test_tools
  {
    namespace tt_detail
    {
      template<> struct print_log_value<fhg::com::p2p::address_t>
      {
        void operator() (std::ostream&, fhg::com::p2p::address_t const&) const;
      };
      template<> struct print_log_value<sdpa::Capability>
      {
        void operator() (std::ostream&, sdpa::Capability const&) const;
      };
    }
  }
}

namespace utils
{
  std::string random_peer_name();

  //! \todo unify with test/layer
  we::type::activity_t module_call (std::string name);

  we::type::activity_t module_call();

  we::type::activity_t net_with_one_child_requiring_workers
    (unsigned long count);

  we::type::activity_t net_with_two_children_requiring_n_workers
    (unsigned long n);

  struct log_to_stdout
  {
    fhg::logging::stdout_sink& sink();
    log_to_stdout (sdpa::daemon::Agent& component);
  };

  class basic_drts_component;

  struct agent
  {
    agent (sdpa::master_network_info, fhg::com::Certificates const&);
    agent (basic_drts_component const& master, fhg::com::Certificates const&);
    agent (agent const& master, fhg::com::Certificates const&);
    agent (fhg::com::Certificates const&);

    agent ( agent const& master_0
          , agent const& master_1
          , fhg::com::Certificates const&
          );

    agent() = delete;
    agent (agent const&) = delete;
    agent (agent&) = delete;
    agent& operator= (agent const&) = delete;
    agent& operator= (agent&) = delete;
    ~agent() = default;

    sdpa::daemon::Agent _;
    log_to_stdout _log_to_stdout = {_};

    std::string name() const;
    fhg::com::host_t host() const;
    fhg::com::port_t port() const;
  };

  using CapabilityNames = std::unordered_set<std::string>;

  // \todo Only used by Restart* tests. Is persistent worker names
  // actually relevant there? If not, remove!
  struct reused_component_name
  {
    //! Only construct with a result from component.name() or
    //! random_peer_name()!
    explicit reused_component_name (std::string name)
      : _name (name)
    {}
    std::string _name;
  };

  class basic_drts_component_no_logic : sdpa::events::EventHandler
  {
  public:
    basic_drts_component_no_logic (fhg::com::Certificates const&);
    basic_drts_component_no_logic
      (reused_component_name, fhg::com::Certificates const&);

    std::string name() const;
    fhg::com::host_t host() const;
    fhg::com::port_t port() const;

  private:
    fhg::util::interruptible_threadsafe_queue
      <std::pair<fhg::com::p2p::address_t, sdpa::events::SDPAEvent::Ptr>>
        _event_queue;

  protected:
    std::string _name;
    sdpa::com::NetworkStrategy _network;

    void event_thread_fun();

    struct event_thread
    {
      event_thread (basic_drts_component_no_logic&);

      basic_drts_component_no_logic& _component;
      boost::strict_scoped_thread<> _event_thread;
      decltype (basic_drts_component_no_logic::_event_queue)::interrupt_on_scope_exit
        _interrupt_thread;
    };
  };

  class basic_drts_component : public basic_drts_component_no_logic
  {
  public:
    basic_drts_component ( bool accept_workers
                         , fhg::com::Certificates const&
                         );
    basic_drts_component ( agent const& master
                         , sdpa::capabilities_set_t
                         , bool accept_workers
                         , fhg::com::Certificates const&
                         );
    basic_drts_component ( agent const& master
                         , CapabilityNames
                         , bool accept_workers
                         , fhg::com::Certificates const&
                         );
    basic_drts_component ( reused_component_name
                         , agent const& master
                         , bool accept_workers
                         , fhg::com::Certificates const&
                         );
    ~basic_drts_component();

    virtual void handle_worker_registration_response
      ( fhg::com::p2p::address_t const&
      , sdpa::events::worker_registration_response const*
      ) override;

    virtual void handleWorkerRegistrationEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::WorkerRegistrationEvent const*
      ) override;

    virtual void handleErrorEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::ErrorEvent const*
      ) override;

    void wait_for_workers_to_shutdown();

  protected:
    boost::optional<fhg::com::p2p::address_t> _master;
    bool _accept_workers;
    std::unordered_set<fhg::com::p2p::address_t> _accepted_workers;

    struct event_thread_and_worker_join
      : basic_drts_component_no_logic::event_thread
    {
      event_thread_and_worker_join (basic_drts_component&);
      ~event_thread_and_worker_join();

      basic_drts_component& _component;
    };

  private:
    std::mutex _mutex_workers_shutdown;
    std::condition_variable _cond_workers_shutdown;
  };

  namespace no_thread
  {
    class basic_drts_worker : public basic_drts_component
    {
    public:
      basic_drts_worker
        ( agent const& master
        , fhg::com::Certificates const&
        );
      basic_drts_worker
        ( agent const& master
        , sdpa::capabilities_set_t
        , fhg::com::Certificates const&
        );
      basic_drts_worker
        ( reused_component_name
        , agent const& master
        , fhg::com::Certificates const&
        );
    };

    class fake_drts_worker_notifying_module_call_submission
      : public basic_drts_worker
    {
    public:
      fake_drts_worker_notifying_module_call_submission
        ( std::function<void (std::string)> announce_job
        , agent const& master
        , fhg::com::Certificates const&
        );

      fake_drts_worker_notifying_module_call_submission
        ( reused_component_name name
        , std::function<void (std::string)>
        , agent const&
        , fhg::com::Certificates const&
        );

      virtual void handleSubmitJobEvent
        ( fhg::com::p2p::address_t const&
        , sdpa::events::SubmitJobEvent const*
        ) override;
      virtual void handleJobFinishedAckEvent
        ( fhg::com::p2p::address_t const&
        , sdpa::events::JobFinishedAckEvent const*
        ) override;

      sdpa::job_id_t job_id (std::string name);

      void add_job ( std::string const& name
                   , sdpa::job_id_t const&
                   , fhg::com::p2p::address_t const& owner
                   );

      void announce_job (std::string const& name);

    protected:
      struct job_t
      {
        sdpa::job_id_t _id;
        fhg::com::p2p::address_t _owner;
        job_t (sdpa::job_id_t id, fhg::com::p2p::address_t owner);
      };
      std::map<std::string, job_t> _jobs;
      void delete_job (sdpa::job_id_t const& job_id);

    private:
      std::function<void (std::string)> _announce_job;
    };

    class fake_drts_worker_waiting_for_finished_ack
      : public no_thread::fake_drts_worker_notifying_module_call_submission
    {
    public:
      fake_drts_worker_waiting_for_finished_ack
        ( std::function<void (std::string)> announce_job
        , agent const& master
        , fhg::com::Certificates const&
        );

      fake_drts_worker_waiting_for_finished_ack
        ( reused_component_name name
        , std::function<void (std::string)> announce_job
        , agent const& master
        , fhg::com::Certificates const&
        );

      virtual void handleJobFinishedAckEvent
        ( fhg::com::p2p::address_t const&
        , sdpa::events::JobFinishedAckEvent const*
        ) override;

      void finish_and_wait_for_ack (std::string name);

    private:
      fhg::util::thread::event<std::string> _finished_ack;
    };
  }

  struct basic_drts_worker final : public no_thread::basic_drts_worker
  {
    basic_drts_worker ( agent const& master
                      , fhg::com::Certificates const&
                      );
    basic_drts_worker ( agent const& master
                      , sdpa::capabilities_set_t
                      , fhg::com::Certificates const&
                      );

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct fake_drts_worker_notifying_module_call_submission final
    : public no_thread::fake_drts_worker_notifying_module_call_submission
  {
    fake_drts_worker_notifying_module_call_submission
      ( std::function<void (std::string)> announce_job
      , agent const& master
      , fhg::com::Certificates const&
      );

    fake_drts_worker_notifying_module_call_submission
      ( reused_component_name name
      , std::function<void (std::string)> announce_job
      , agent const& master
      , fhg::com::Certificates const&
      );

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct fake_drts_worker_waiting_for_finished_ack final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
    fake_drts_worker_waiting_for_finished_ack
      ( std::function<void (std::string)> announce_job
      , agent const& master
      , fhg::com::Certificates const&
      );

    fake_drts_worker_waiting_for_finished_ack
      ( reused_component_name name
      , std::function<void (std::string)> announce_job
      , agent const& master
      , fhg::com::Certificates const&
      );

    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class fake_drts_worker_notifying_cancel final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
  public:
    fake_drts_worker_notifying_cancel
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& master
      , fhg::com::Certificates const&
      );

    void handleCancelJobEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::CancelJobEvent const*
      ) override;

    void canceled (std::string job_id);

  private:
    std::function<void (std::string)> _announce_cancel;
    mutable std::mutex _cancels_mutex;
    std::map<std::string, fhg::com::p2p::address_t> _cancels;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  class fake_drts_worker_notifying_cancel_but_never_replying final
    : public no_thread::fake_drts_worker_waiting_for_finished_ack
  {
  public:
    fake_drts_worker_notifying_cancel_but_never_replying
      ( std::function<void (std::string)> announce_job
      , std::function<void (std::string)> announce_cancel
      , agent const& master
      , fhg::com::Certificates const&
      );

    void handleCancelJobEvent
      ( fhg::com::p2p::address_t const&
      , sdpa::events::CancelJobEvent const*
      ) override;

  private:
    std::function<void (std::string)> _announce_cancel;
    basic_drts_component::event_thread_and_worker_join _ = {*this};
  };

  struct client : boost::noncopyable
  {
    client (agent const&, fhg::com::Certificates const&);

    client() = delete;
    client (client const&) = delete;
    client (client&) = delete;
    client& operator= (client const&) = delete;
    client& operator= (client&) = delete;
    ~client() = default;

    sdpa::job_id_t submit_job (we::type::activity_t);

    sdpa::status::code query_job_status (sdpa::job_id_t const&);

    sdpa::status::code wait_for_terminal_state (sdpa::job_id_t const&);
    sdpa::status::code wait_for_terminal_state
      (sdpa::job_id_t const&, sdpa::client::job_info_t& job_info);

    sdpa::status::code wait_for_terminal_state_and_cleanup
      (sdpa::job_id_t const&);

    sdpa::discovery_info_t discover (sdpa::job_id_t const&);

    void delete_job (sdpa::job_id_t const&);

    void cancel_job (sdpa::job_id_t const&);

    sdpa::client::Client _;

    static sdpa::status::code submit_job_and_wait_for_termination_as_subscriber
      ( we::type::activity_t
      , agent const&
      , fhg::com::Certificates const&
      );
  };
}
