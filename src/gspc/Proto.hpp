#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>

#include <gspc/job/ID.hpp>

#include <gspc/remote_interface/Hostname.hpp>
#include <gspc/remote_interface/ID.hpp>

#include <gspc/resource/Class.hpp>
#include <gspc/resource/ID.hpp>

#include <gspc/rpc/TODO.hpp>

#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>

#include <gspc/value_type.hpp>

#include <gspc/util-generic_hash_forward_declare.hpp>
#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/hash/combined_hash.hpp>
#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/this_bound_mem_fn.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <boost/variant.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/filesystem/path.hpp>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  //! between Scheduler and Worker
  struct Job
  {
    //! \todo multiple tasks per job!?
    Task task;

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & task;
    }
  };

  namespace job
  {
    namespace finish_reason
    {
      struct Finished
      {
        //! \todo multiple tasks per job!?
        ErrorOr<task::Result> task_result;
        template<typename Archive>
          void serialize (Archive& ar, unsigned int)
        {
          ar & task_result;
        }
      };
      struct WorkerFailure
      {
        std::exception_ptr exception;
        template<typename Archive>
          void serialize (Archive& ar, unsigned int)
        {
          ar & exception;
        }
      };
      struct Cancelled
      {
        template<typename Archive>
          void serialize (Archive&, unsigned int)
        {}
      };
    }

    //! \todo Is worker failure a finish though?
    using FinishReason = boost::variant < finish_reason::Finished
                                        , finish_reason::WorkerFailure
                                        , finish_reason::Cancelled
                                        >;
  }
}

namespace gspc
{
  namespace interface
  {
    class ResourceManager;
  }

  namespace comm
  {
    namespace runtime_system
    {
      namespace resource_manager
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          ( add
          , void (Forest<resource::ID, resource::Class>)
          );
        FHG_RPC_FUNCTION_DESCRIPTION
          ( remove
          , void (Forest<resource::ID>)
          );

        using Client = interface::ResourceManager&;
        struct Server{};
      }
    }

    namespace scheduler
    {
      namespace worker
      {
        //! \todo worker::scheduler::Server::endpoint?
        FHG_RPC_FUNCTION_DESCRIPTION
          ( submit
          , void (rpc::endpoint, job::ID, Job)
          );
        FHG_RPC_FUNCTION_DESCRIPTION
          ( cancel
          , void (job::ID)
          );
        // status

        struct Client
        {
        public:
          Client (boost::asio::io_service&, rpc::endpoint);

        private:
          std::unique_ptr<rpc::remote_endpoint> _endpoint;

        public:
          rpc::sync_remote_function<worker::submit> submit;
          rpc::sync_remote_function<worker::cancel> cancel;
        };

        struct Server
        {
        private:
          rpc::service_dispatcher _service_dispatcher;
          fhg::util::scoped_boost_asio_io_service_with_threads _io_service;

        public:
          rpc::service_handler<worker::submit> const _submit;
          rpc::service_handler<worker::cancel> const _cancel;

        private:
          rpc::service_socket_provider const _service_socket_provider;
          rpc::service_tcp_provider const _service_tcp_provider;
          rpc::endpoint const _local_endpoint;

        public:
          template<typename Submit, typename Cancel>
            Server (Submit&&, Cancel&&);
          template<typename That>
            Server (That*);

          rpc::endpoint local_endpoint() const;
        };
      }
    }

    namespace worker
    {
      namespace scheduler
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          ( finished
          , void (job::ID, job::FinishReason)
          );

        struct Client
        {
        public:
          Client (boost::asio::io_service&, rpc::endpoint);

        private:
          std::unique_ptr<rpc::remote_endpoint> _endpoint;

        public:
          rpc::sync_remote_function<scheduler::finished> finished;
        };

        struct Server
        {
        private:
          rpc::service_dispatcher _service_dispatcher;
          fhg::util::scoped_boost_asio_io_service_with_threads _io_service;

        public:
          rpc::service_handler<scheduler::finished> const _finished;

        private:
          rpc::service_socket_provider const _service_socket_provider;
          rpc::service_tcp_provider const _service_tcp_provider;
          rpc::endpoint const _local_endpoint;

        public:
          template<typename Finished>
            Server (Finished&&);
          template<typename That>
            Server (That*);

          rpc::endpoint local_endpoint() const;
        };
      }
    }
  }
}

namespace gspc
{
  namespace workflow_engine
  {
    struct ProcessingState
    {
    public:
      Task extract
        ( resource::Class resource_class
        , std::unordered_map<std::string, value_type> inputs
        , boost::filesystem::path so
        , std::string symbol
        );

      template<typename Function>
        using is_post_process =
          fhg::util::is_callable< Function
                                , void (Task const&, task::Result const&)
                                >;

      template< typename Function
              , typename = std::enable_if_t<is_post_process<Function>{}>
              >
      void inject
        ( task::ID task_id
        , ErrorOr<task::Result> error_or_result
        , Function&& post_process_result
        );

      bool has_extracted_tasks() const;

      friend std::ostream& operator<< (std::ostream&, ProcessingState const&);

    private:
      task::ID _next_task_id {0};

      //! every task in tasks is either
      //! - in flight: _extracted
      //! - failed executing: _failed_to_execute
      //! - failed to post process: _failed_to_post_process
      //! note: finished tasks are forgotten (they changed state when
      //! post processed).
      //! \note would be enough to remember data required to reconstruct task
      std::unordered_map<task::ID, Task> _tasks;

      //! \note keys of _extracted, _failed_to_post_process,
      //! _failed_to_execute are
      //! - disjoint
      //! - subset of keys (_tasks)
      std::unordered_set<task::ID> _extracted;
      std::unordered_map< task::ID
                        , std::pair<task::Result, std::exception_ptr>
                        > _failed_to_post_process;
      std::unordered_map<task::ID, std::exception_ptr> _failed_to_execute;
    };

    struct State
    {
      friend std::ostream& operator<< (std::ostream&, State const&);

      //! \todo ctor
      // private:
      std::vector<char> engine_specific;
      bool workflow_finished;

      ProcessingState processing_state;
    };
  }
}

namespace gspc
{
  namespace interface
  {
    //! \todo what is base class, what is implementation specific!?
    //! likely: _resources, _resource_usage_by_id, add+remove base,
    //! NOT available_by_x
    //! _resources -> what about the lock
    //! _resource_usage_by_id -> is ref counting always needed?
    //! e.g. what if assert_singletons_only
    //! add+remove -> into/from what state?
    //! Alternative: Factor shared states
    class ResourceManager
    {
    public:
      using Resources = Forest<resource::ID, resource::Class>;

      virtual ~ResourceManager() = default;

      // - shall throw on duplicate id
      virtual void add (Resources) = 0;
      virtual void remove (Forest<resource::ID>) = 0;

      struct Interrupted : public std::exception{};
      //! \note once called all running and future acquire will throw
      //! Interrupted
      //! \todo Discuss this implies that a single resource manager can not be
      //! shared between multiple clients
      virtual void interrupt() = 0;
      //! \todo maybe return optional<...> instead of throwing Interrupted
      // virtual ? acquire (?)
      // virtual void release (?)
    private:
      //! \todo
      // comm::runtime_system::resource_manager::Server _comm_server_for_runtime_system;
    };

    class Scheduler
    {
    public:
      template<typename Derived>
        Scheduler (Derived*);
      virtual ~Scheduler() = default;

      // called by user
      virtual void wait() = 0;
      virtual void stop() = 0;

      //! \todo Do we require the called to also know they have to call
      //! the workflow engine or should we automatically query that here
      //! as well? Effectively this is we.status().extracted_tasks +
      //! their state.
      //      virtual std::unordered_map<job::Description, job::State> status() const;

      // called by worker
      virtual void finished (job::ID, job::FinishReason) = 0;

    protected:
      fhg::util::scoped_boost_asio_io_service_with_threads
        _io_service_for_workers {1};
      comm::worker::scheduler::Server const _comm_server_for_worker;
    };

    class WorkflowEngine
    {
    public:
      virtual ~WorkflowEngine() = default;
      //! extract.is_bool && extract.second == false: workflow has not
      //! finished yet, e.g. because it waits for external events
      //! \note workflow engine shall not say "true" when there are
      //! extacted tasks
      virtual boost::variant<Task, bool> extract() = 0;
      virtual void inject (task::ID, ErrorOr<task::Result>) = 0;


      virtual workflow_engine::State state() const = 0;
      //! \required: WorkflowEngine (workflow_engine::State)

      // virtual std::vector<char> dump() const;
    };
  }

  class PetriNetWorkflow{};
  class PetriNetWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    PetriNetWorkflowEngine (PetriNetWorkflow);

    virtual boost::variant<Task, bool> extract() override;
    virtual void inject (task::ID, ErrorOr<task::Result>) override;
  };
  class MapWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    MapWorkflowEngine (std::uint64_t);

    virtual boost::variant<Task, bool> extract() override;
    virtual void inject (task::ID, ErrorOr<task::Result>) override;

    virtual workflow_engine::State state() const override;
    MapWorkflowEngine (workflow_engine::State);

  private:
    struct
    {
      std::uint64_t N;
      std::uint64_t i {0};

      template<typename Archive>
        void serialize (Archive& ar, unsigned int /* version */)
      {
        ar & N;
        ar & i;
      }
    } _workflow_state;

    bool workflow_finished() const;

    workflow_engine::ProcessingState _processing_state;
  };
  class TreeTraversalWorkflow;
  class TreeTraversalWorkflowEngine;
  class MapReduceWorkflow;
  class MapReduceWorkflowEngine;

  class ReschedulingGreedyScheduler;
  class LookaheadScheduler;
  class WorkStealingScheduler;
  class CoallocationScheduler;
  class TransferCostAwareScheduler;

  namespace resource_manager
  {
    class WithPreferences : public interface::ResourceManager
    {
    public:
      virtual void add (Resources) override;
      virtual void remove (Forest<resource::ID>) override;

      struct Acquired
      {
        resource::ID requested;
        //! \note not shown to the user but implicitly locked, in
        //! order to avoid partial release. note: disallows freeing
        //! only requested but keeping dependent, e.g. (A -> B,
        //! request B, get {B, A}, release only B, still have A.)
        // std::unordered_set<resource::ID> dependent;
      };

      //! blocks if no resource of that class available/exists
      //! (not-block-on-not-exists would race with add).
      Acquired acquire (std::list<resource::Class>);
      void release (Acquired const&);
      virtual void interrupt() override;

    private:
      std::mutex _resources_guard;
      std::condition_variable _resources_became_available_or_interrupted;
      bool _interrupted {false};

      Resources _resources;
      std::unordered_map<resource::ID, std::size_t> _resource_usage_by_id;
      std::unordered_map<resource::Class, std::unordered_set<resource::ID>>
        _available_resources_by_class;

      void release (resource::ID const&);
    };

    class Trivial : public WithPreferences
    {
    public:
      Acquired acquire (resource::Class resource_class)
      {
        return WithPreferences::acquire ({resource_class});
      }
    };

    class Coallocation : public interface::ResourceManager
    {
    public:
      //! \note Only supports forward-disjoint classes.
      virtual void add (Resources) override;
      virtual void remove (Forest<resource::ID>) override;

      struct Acquired
      {
        std::unordered_set<resource::ID> requesteds;
        //! \note not shown to the user but implicitly locked, in
        //! order to avoid partial release. note: disallows freeing
        //! only requested but keeping dependent, e.g. (A -> B,
        //! request B, get {B, A}, release only B, still have A.)
        // std::unordered_set<resource::ID> dependent;
      };

      Acquired acquire (resource::Class, std::size_t);
      void release (Acquired const&);
      virtual void interrupt() override;

    private:
      //! C is not forward disjoint: C -> D <- C, but C -> D <- B is,
      //! also transitive! No acquire of a C-class resource shall
      //! implicitly block a different C-class resource.
      static void assert_is_strictly_forward_disjoint_by_resource_class
        (Resources const&);

      std::mutex _resources_guard;
      std::condition_variable _resources_became_available_or_interrupted;
      bool _interrupted {false};

      Resources _resources;
      std::unordered_map<resource::ID, std::size_t> _resource_usage_by_id;
      std::unordered_map<resource::Class, std::unordered_set<resource::ID>>
        _available_resources_by_class;

      void release (resource::ID const&);
    };

    // class CoallocationWithPreference : public interface::ResourceManager
    // {
    //   // `[(rc, count)]` or `[rc], count` or both?
    //   Acquired acquire (std::list<std::pair<resource::Class, std::size_t>>);
    // };
  }
}

namespace gspc
{
  class Resource
  {
  public:
    //! \todo individual-worker-specific, non-resource attributes,
    //! e.g. socket binding, port, memory…
    resource::Class resource_class;

    template<typename Archive> void serialize (Archive& ar, unsigned int)
    {
      ar & resource_class;
    }
  };

  bool operator== (Resource const&, Resource const&);
  std::ostream& operator<< (std::ostream&, Resource const&);
}

UTIL_MAKE_COMBINED_STD_HASH_DECLARE (gspc::Resource);

namespace gspc
{
  namespace comm
  {
    namespace runtime_system
    {
      namespace remote_interface
      {
        FHG_RPC_FUNCTION_DESCRIPTION
          ( add
          , UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
              (UniqueForest<Resource>)
          );

        FHG_RPC_FUNCTION_DESCRIPTION
          ( remove
          , Forest<resource::ID, ErrorOr<>> (Forest<resource::ID>)
          );

        FHG_RPC_FUNCTION_DESCRIPTION
          ( worker_endpoint_for_scheduler
          , rpc::endpoint (resource::ID)
          );

        //! \todo syntax goal:
        //! RPC_CLIENT (Client, add, remove, worker_endpoint_for_scheduler);
        //! RPC_SERVER (Server, add, remove, worker_endpoint_for_scheduler);
        struct Client
        {
        public:
          Client (boost::asio::io_service&, rpc::endpoint);

        private:
          std::unique_ptr<rpc::remote_endpoint> _endpoint;

        public:
          rpc::sync_remote_function<remote_interface::add> add;
          rpc::sync_remote_function<remote_interface::remove> remove;
          rpc::sync_remote_function
            <remote_interface::worker_endpoint_for_scheduler>
              worker_endpoint_for_scheduler;
        };
      }
    }
    namespace scheduler
    {
      namespace workflow_engine
      {
        // next
        // inject
        // extract

        using Client = interface::WorkflowEngine&;
      }
      namespace resource_manager
      {
        // acquire
        // release
        // interrupt
      }
    }
    namespace user
    {
      namespace workflow_engine
      {
        // dump_state
      }
      namespace scheduler
      {
        // wait
        // stop
        // status
      }
      namespace runtime_system
      {
        // add
        // remove
      }
    }
  }

  class Worker
  {
  public:
    Worker (Resource);

    Worker (Worker const&) = delete;
    Worker (Worker&&) = delete;
    Worker& operator= (Worker const&) = delete;
    Worker& operator= (Worker&&) = delete;
    //! \note dtor waits? cancels?
    ~Worker();

    rpc::endpoint endpoint_for_scheduler() const;

    // called by scheduler (via rif)
    void submit (rpc::endpoint, job::ID, Job);
    void cancel (job::ID);

    // called by user via scheduler (via rif)
    // TaskState status (job::ID);

  private:
    Resource _resource;

    fhg::util::scoped_boost_asio_io_service_with_threads
      _io_service_for_scheduler {1};
    comm::scheduler::worker::Server const _comm_server_for_scheduler;

    //! \todo name: Job? Task?
    struct WorkItem
    {
      rpc::endpoint scheduler;
      job::ID job_id;
      Job job;
    };
    using WorkQueue = fhg::util::interruptible_threadsafe_queue<WorkItem>;
    WorkQueue _work_queue;
    std::thread _worker_thread;
    void work();
  };

  class RemoteInterface
  {
  public:
    RemoteInterface (remote_interface::ID);

    UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
      add (UniqueForest<Resource> const&);

    Forest<resource::ID, ErrorOr<>> remove (Forest<resource::ID> const&);

    rpc::endpoint const& local_endpoint() const;
    rpc::endpoint worker_endpoint_for_scheduler (resource::ID) const;

  private:
    resource::ID _next_resource_id;

    //! BEGIN Syntax goal:
    //! comm::runtime_system::remote_interface::Server _comm_server;
    rpc::service_dispatcher _service_dispatcher;
    fhg::util::scoped_boost_asio_io_service_with_threads _io_service;

    rpc::service_handler<comm::runtime_system::remote_interface::add> const _add;
    rpc::service_handler<comm::runtime_system::remote_interface::remove> const _remove;
    rpc::service_handler<comm::runtime_system::remote_interface::worker_endpoint_for_scheduler> const _worker_endpoint_for_scheduler;

    rpc::service_socket_provider const _service_socket_provider;
    rpc::service_tcp_provider const _service_tcp_provider;
    rpc::endpoint const _local_endpoint;
    //! END Syntax goal

    //! \note process proxy
    struct WorkerServer
    {
      WorkerServer (Resource const&);

      rpc::endpoint endpoint_for_scheduler() const;

    private:
      Worker _worker;
    };

    std::unordered_map<resource::ID, WorkerServer> _workers;
  };

  namespace remote_interface
  {
    namespace strategy
    {
      // class ssh;

      class Thread
      {
        struct RemoteInterfaceServer
        {
          RemoteInterfaceServer (ID);

          rpc::endpoint local_endpoint() const;

        private:
          RemoteInterface _remote_interface;
        };

      public:
        using State = std::unordered_map<Hostname, RemoteInterfaceServer>;

        Thread (State*);
        rpc::endpoint boot (Hostname, ID) const;
        void teardown (Hostname) const;

      private:
        State* _remote_interfaces_by_hostname;
      };
    }

    //! Strategy must not contain state but only information
    //! (e.g. pointer, filename, ...) about to get a state, Strategies
    //! _are_ copied!
    using Strategy = boost::variant< strategy::Thread
                                   // , strategy::ssh
                                   >;

    class ConnectionAndPID
    {
    public:
      ConnectionAndPID ( boost::asio::io_service&
                       , Hostname
                       , Strategy
                       , ID
                       );
      ~ConnectionAndPID();

      ConnectionAndPID() = delete;
      ConnectionAndPID (ConnectionAndPID const&) = delete;
      ConnectionAndPID (ConnectionAndPID&&) = delete;
      ConnectionAndPID& operator= (ConnectionAndPID const&) = delete;
      ConnectionAndPID& operator= (ConnectionAndPID&&) = delete;

      UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
        add (UniqueForest<Resource> const&);
      Forest<resource::ID, ErrorOr<>> remove (Forest<resource::ID> const&);

      rpc::endpoint worker_endpoint_for_scheduler (resource::ID);

      Strategy const& strategy() const;

    private:
      Hostname _hostname;
      Strategy _strategy;
      comm::runtime_system::remote_interface::Client _client;
    };
  }
}

namespace gspc
{
  //! \todo Scoped because ConnectionAndPID is scoped. Is unscoped
  //! actually needed?
  class ScopedRuntimeSystem
  {
  public:
    ScopedRuntimeSystem (comm::runtime_system::resource_manager::Client);

    std::unordered_map
      < remote_interface::Hostname
      , ErrorOr<UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>>
      >
      add ( std::unordered_set<remote_interface::Hostname>
          , remote_interface::Strategy
          , UniqueForest<Resource> const&
          ) noexcept;

    Forest<resource::ID>
      add_or_throw  ( std::unordered_set<remote_interface::Hostname> hostnames
                    , remote_interface::Strategy strategy
                    , UniqueForest<Resource> const& resources
                    );

    //! \note Assumes that two connected resource IDs have the same
    //! RemoteInterface ID.
    Forest<resource::ID, ErrorOr<>> remove (Forest<resource::ID> const&);

    rpc::endpoint worker_endpoint_for_scheduler (resource::ID) const;

  private:
    comm::runtime_system::resource_manager::Client _resource_manager;

    //! \todo thread count based on parameter or?
    fhg::util::scoped_boost_asio_io_service_with_threads
      _remote_interface_io_service {1};

    remote_interface::ID _next_remote_interface_id;

    //! \todo cleanup, e.g. when last resource using them is
    //! removed.
    std::unordered_map< remote_interface::Hostname
                      , std::unique_ptr<remote_interface::ConnectionAndPID>
                      > _remote_interface_by_hostname;
    std::unordered_map< remote_interface::ID
                      , remote_interface::Hostname
                      > _hostname_by_remote_interface_id;

    remote_interface::ConnectionAndPID*
      remote_interface_by_id (remote_interface::ID) const;

    std::unordered_map
      < remote_interface::Hostname
      , ErrorOr<remote_interface::ConnectionAndPID*>
      >
      remote_interfaces ( std::unordered_set<remote_interface::Hostname>
                        , remote_interface::Strategy
                        ) noexcept;

    //! \todo OPTIMIZE access to hostname via map<hostID, hostName>
  };
}

namespace gspc
{
  class GreedyScheduler : public interface::Scheduler
  {
  public:
    //! \note: workflow_engine can not be shared with other schedulers
    //! because alien inject would break the finish condition

    //! \todo also applies to put_token: either managed by scheduler
    //! or requires notification from workflow_engine to
    //! scheduler. Possible change in API: let
    //! workflow_engine::extract() block
    GreedyScheduler ( comm::scheduler::workflow_engine::Client
                    , resource_manager::Trivial& //! \todo: Client
                    , ScopedRuntimeSystem& //! \todo UnscopedBase
                    );

    virtual void wait() override;
    virtual void stop() override;
    virtual void finished (job::ID, job::FinishReason) override;

  private:
    comm::scheduler::workflow_engine::Client _workflow_engine;
    resource_manager::Trivial& _resource_manager;
    ScopedRuntimeSystem& _runtime_system;

    std::atomic<bool> _stopped {false};

    std::mutex _guard_state;
    std::condition_variable _injected_or_stopped;
    bool _injected {false};
    std::unordered_map<task::ID, resource::ID> _tasks;

    std::thread _thread;
    void scheduling_thread();
  };
}
