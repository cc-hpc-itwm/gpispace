#pragma once

#include <gspc/comm/runtime_system/resource_manager/Client.hpp>
#include <gspc/comm/runtime_system/resource_manager/Server.hpp>
#include <gspc/comm/worker/scheduler/Server.hpp>

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>

#include <gspc/interface/ResourceManager.hpp>
#include <gspc/interface/Scheduler.hpp>
#include <gspc/interface/WorkflowEngine.hpp>

#include <gspc/Job.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/ID.hpp>

#include <gspc/remote_interface/Hostname.hpp>
#include <gspc/remote_interface/ID.hpp>

#include <gspc/Resource.hpp>
#include <gspc/resource/Class.hpp>
#include <gspc/resource/ID.hpp>

#include <gspc/resource_manager/Coallocation.hpp>
#include <gspc/resource_manager/Trivial.hpp>
#include <gspc/resource_manager/WithPreferences.hpp>

#include <gspc/rpc/TODO.hpp>

#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>

#include <gspc/value_type.hpp>

#include <gspc/Worker.hpp>

#include <gspc/workflow_engine/ProcessingState.hpp>
#include <gspc/workflow_engine/State.hpp>

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
    // class CoallocationWithPreference : public interface::ResourceManager
    // {
    //   // `[(rc, count)]` or `[rc], count` or both?
    //   Acquired acquire (std::list<std::pair<resource::Class, std::size_t>>);
    // };
  }
}

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
