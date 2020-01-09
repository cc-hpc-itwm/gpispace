#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>

#include <logging/endpoint.hpp>

#include <rpc/function_description.hpp>
#include <rpc/remote_endpoint.hpp>
#include <rpc/remote_function.hpp>
#include <rpc/remote_socket_endpoint.hpp>
#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/hash/combined_hash.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/this_bound_mem_fn.hpp>

#include <boost/variant.hpp>
#include <boost/range/adaptor/map.hpp>

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

//! \todo merge rpc/logging
namespace rpc
{
  using endpoint = fhg::logging::endpoint;
  using tcp_endpoint = fhg::logging::tcp_endpoint;
  using socket_endpoint = fhg::logging::socket_endpoint;
  using remote_endpoint = fhg::rpc::remote_endpoint;
  using remote_tcp_endpoint = fhg::rpc::remote_tcp_endpoint;
  using remote_socket_endpoint = fhg::rpc::remote_socket_endpoint;
  using service_dispatcher = fhg::rpc::service_dispatcher;
  using service_socket_provider = fhg::rpc::service_socket_provider;
  using service_tcp_provider = fhg::rpc::service_tcp_provider;

  template<typename Protocol>
    using service_handler = fhg::rpc::service_handler<Protocol>;
  template<typename Protocol>
    using remote_function = fhg::rpc::remote_function<Protocol>;

  std::unique_ptr<remote_endpoint> make_endpoint
    (boost::asio::io_service& io_service, endpoint);
}

namespace gspc
{
  namespace interface
  {
    class ResourceManager;
    class Scheduler;
    class WorkflowEngine;
  }

  class PetriNetWorkflow{};
  class PetriNetWorkflowEngine
  {
  public:
    PetriNetWorkflowEngine (PetriNetWorkflow);
  };
  class TreeTraversalWorkflow;
  class TreeTraversalWorkflowEngine;
  class MapReduceWorkflow;
  class MapReduceWorkflowEngine;

  class GreedyScheduler
  {
  public:
    template<typename WE, typename RM, typename RTS>
      GreedyScheduler (WE&, RM&, RTS const&);

    void wait();
  };
  class ReschedulingGreedyScheduler;
  class LookaheadScheduler;
  class WorkStealingScheduler;
  class CoallocationScheduler;
  class TransferCostAwareScheduler;

  namespace resource_manager
  {
    class Trivial{};
    class Coallocation;
    class WithPreferences;
  }
}

namespace gspc
{
  class Resource
  {
  public:
    int _;

    template<typename Archive> void serialize (Archive&, unsigned int) {}
  };

  bool operator== (Resource const&, Resource const&);
}

FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::Resource
  , r
  , r._
  )

namespace gspc
{
  namespace remote_interface
  {
    struct ID
    {
      std::uint64_t id {0};

      ID& operator++() { ++id; return *this; }

      template<typename Archive>
        void serialize (Archive& ar, unsigned int)
      {
        ar & id;
      }
    };
    bool operator== (ID const& lhs, ID const& rhs);
    std::ostream& operator<< (std::ostream&, ID const&);

    using Hostname = std::string;
  }
}

FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::remote_interface::ID
  , x
  , x.id
  )

namespace gspc
{
  namespace resource
  {
    struct ID
    {
      ID (remote_interface::ID rif) : remote_interface (rif), id (0) {}

      remote_interface::ID remote_interface;
      std::uint64_t id;

      ID& operator++() { ++id; return *this; }

      //! \note for serialization
      ID() = default;
      template<typename Archive>
        void serialize (Archive& ar, unsigned int)
      {
        ar & remote_interface;
        ar & id;
      }
    };
    bool operator== (ID const& lhs, ID const& rhs);
    std::ostream& operator<< (std::ostream&, ID const&);
  }
}

  FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::resource::ID
  , x
  , x.id
  , x.remote_interface
  )

namespace gspc
{
  namespace remote_interface
  {
    namespace protocol
    {
      FHG_RPC_FUNCTION_DESCRIPTION
        ( add
        , Forest<Resource, ErrorOr<resource::ID>>
            (Forest<Resource> /* \todo RPC: const& */)
        );
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
    ~Worker() = default;

    rpc::endpoint const& local_endpoint() const;

  private:
    Resource _resource;

    rpc::service_dispatcher _service_dispatcher;
    fhg::util::scoped_boost_asio_io_service_with_threads _io_service;

    // rpc::service_handler<worker::protocol::submit> const _submit;

    rpc::service_socket_provider const _service_socket_provider;
    rpc::service_tcp_provider const _service_tcp_provider;
    rpc::endpoint const _local_endpoint;
  };

  class RemoteInterface
  {
  public:
    RemoteInterface (remote_interface::ID);

    Forest<Resource, ErrorOr<resource::ID>>
      add (Forest<Resource> const&);

    //! \todo is unordered_set<resource::ID> enough? instead of
    //! Forest<resource::id>? because ther traversal is unordered
    //! anyways
    Forest<resource::ID, ErrorOr<>> remove (Forest<resource::ID> const&);

    rpc::endpoint const& local_endpoint() const;

    //! \todo or alternative: be a proxy and forward to worker
    //    rpc::endpoint const& local_endpoint (resource::ID) const;

  private:
    resource::ID _next_resource_id;

    rpc::service_dispatcher _service_dispatcher;
    fhg::util::scoped_boost_asio_io_service_with_threads _io_service;

    rpc::service_handler<remote_interface::protocol::add> const _add;

    rpc::service_socket_provider const _service_socket_provider;
    rpc::service_tcp_provider const _service_tcp_provider;
    rpc::endpoint const _local_endpoint;

    //! \note process proxy
    struct WorkerServer
    {
      WorkerServer (Resource const&);

      rpc::endpoint local_endpoint() const;

    private:
      Worker _worker;
    };

    std::unordered_map<resource::ID, WorkerServer> _workers;
  };

  namespace remote_interface
  {
    struct RuntimeSystemToRemoteInterface
    {
    public:
      RuntimeSystemToRemoteInterface (boost::asio::io_service&, rpc::endpoint);

    private:
      std::unique_ptr<rpc::remote_endpoint> _endpoint;

    public:
      rpc::remote_function<protocol::add> add;
    };

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

      Forest<Resource, ErrorOr<resource::ID>>
        add (Forest<Resource> const&);

      Strategy const& strategy() const;

    private:
      Hostname _hostname;
      Strategy _strategy;
      RuntimeSystemToRemoteInterface _client;
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
    template<typename RM> ScopedRuntimeSystem (RM&);

    std::unordered_map
      < remote_interface::Hostname
      , ErrorOr<Forest<Resource, ErrorOr<resource::ID>>>
      >
      add ( std::unordered_set<remote_interface::Hostname>
          , remote_interface::Strategy
          , Forest<Resource> const&
          ) noexcept;

    Forest<resource::ID>
      add_or_throw  ( std::unordered_set<remote_interface::Hostname> hostnames
                    , remote_interface::Strategy strategy
                    , Forest<Resource> const& resources
                    );

    //! \todo return value and noexcept!? is Forest<resource::ID>
    //! required? Or would unordered_set be enough?
    void remove (Forest<resource::ID> const&);

  private:
    //! \todo thread count based on parameter or?
    fhg::util::scoped_boost_asio_io_service_with_threads
      _remote_interface_io_service {1};

    remote_interface::ID _next_remote_interface_id;

    std::unordered_map< remote_interface::Hostname
                      , remote_interface::ConnectionAndPID
                      > _remote_interface_by_hostname;
    //! \todo
    // std::unordered_map< resource::ID
    //                   , remote_interface::Hostname
    //                   > _hostname_by_resource_id;

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
