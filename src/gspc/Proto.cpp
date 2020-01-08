#include <gspc/util/Forest.hpp>
#include <gspc/MaybeError.hpp>

#include <logging/endpoint.hpp>

#include <rpc/remote_endpoint.hpp>
#include <rpc/service_dispatcher.hpp>
#include <rpc/service_handler.hpp>
#include <rpc/service_socket_provider.hpp>
#include <rpc/service_tcp_provider.hpp>

#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/hash/combined_hash.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <boost/variant.hpp>
#include <boost/range/adaptor/map.hpp>

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unordered_map>
#include <unordered_set>

//! \todo merge rpc/logging
namespace rpc
{
  using endpoint = fhg::logging::endpoint;
  using remote_endpoint = fhg::rpc::remote_endpoint;
  using service_dispatcher = fhg::rpc::service_dispatcher;
  using service_socket_provider = fhg::rpc::service_socket_provider;
  using service_tcp_provider = fhg::rpc::service_tcp_provider;

  template<typename Protocol>
    using service_handler = fhg::rpc::service_handler<Protocol>;
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

template<typename, typename A> using AnnotatedForest
  = std::list<std::pair<int, A>>;

namespace gspc
{
  class Resource
  {
  public:
    int _;
  };

  bool operator== (Resource const&, Resource const&);

  namespace remote_interface
  {
    struct ID
    {
      std::uint64_t id;

      ID& operator++() { ++id; return *this; }
    };
    bool operator== (ID const&, ID const&);

    using Hostname = std::string;
  }

  namespace resource
  {
    struct ID
    {
      remote_interface::ID remote_interface;
      std::uint64_t id;
    };
    bool operator== (ID const&, ID const&);
  }

  class RemoteInterface
  {
  public:
    RemoteInterface (remote_interface::ID);

    AnnotatedForest<Resource, MaybeError<resource::ID>>
      add (util::Forest<Resource>);

  private:
    remote_interface::ID _id;
  };

  namespace remote_interface
  {
    struct RuntimeSystemToRemoteInterface
    {
      RuntimeSystemToRemoteInterface (rpc::endpoint);

      AnnotatedForest<Resource, MaybeError<resource::ID>>
        add (util::Forest<Resource> const&);
    };

    namespace strategy
    {
      // class ssh;

      class Thread
      {
        struct RemoteInterfaceServer
        {
          RemoteInterfaceServer (ID);

          rpc::endpoint endpoint() const;

        private:
          RemoteInterface _remote_interface;
          rpc::service_dispatcher _service_dispatcher;
          fhg::util::scoped_boost_asio_io_service_with_threads _io_service;

          // rpc::service_handler<protocol::register_receiver> const
          //   _register_receiver;

          rpc::service_socket_provider const _service_socket_provider;
          rpc::service_tcp_provider const _service_tcp_provider;
          rpc::endpoint const _local_endpoint;
        };

      public:
        using State = std::unordered_map<Hostname, RemoteInterfaceServer>;

        Thread (State*);
        RuntimeSystemToRemoteInterface boot (Hostname, ID) const;
        void teardown (Hostname) const;

      private:
        State* _remote_interfaces_by_hostname;
      };
    }

    using Strategy = boost::variant< strategy::Thread
                                   // , strategy::ssh
                                   >;

    class ConnectionAndPID
    {
    public:
      ConnectionAndPID ( Hostname
                       , Strategy
                       , ID
                       );

      AnnotatedForest<Resource, MaybeError<resource::ID>>
        add (util::Forest<Resource> const&);

    private:
      Hostname _hostname;
      Strategy _strategy;
      RuntimeSystemToRemoteInterface _client;
    };
  }
}

FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::remote_interface::ID
  , x
  , x.id
  );
FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::resource::ID
  , x
  , x.id
  , x.remote_interface
  );
FHG_UTIL_MAKE_COMBINED_STD_HASH
  ( gspc::Resource
  , r
  , r._
  );

namespace gspc
{
  class RuntimeSystem
  {
  public:
    template<typename RM> RuntimeSystem (RM&);

    std::unordered_map
      < remote_interface::Hostname
      , MaybeError<AnnotatedForest<Resource, MaybeError<resource::ID>>>
      >
      add ( std::unordered_set<remote_interface::Hostname>
          , remote_interface::Strategy
          , util::Forest<Resource>
          ) noexcept;

    std::unordered_set<resource::ID>
      add_or_throw  ( std::unordered_set<remote_interface::Hostname> hostnames
                    , remote_interface::Strategy strategy
                    , util::Forest<Resource> resources
                    );

    //! \todo return value and noexcept!?
    void remove (std::unordered_set<resource::ID>);

  private:
    remote_interface::ID _next_remote_interface_id {0};

    std::unordered_map< remote_interface::Hostname
                      , remote_interface::ConnectionAndPID
                      > _remote_interface_by_hostname;
    std::unordered_map< resource::ID
                      , remote_interface::Hostname
                      > _hostname_by_resource_id;

    std::unordered_map
      < remote_interface::Hostname
      , MaybeError<remote_interface::ConnectionAndPID*>
      >
      remote_interfaces ( std::unordered_set<remote_interface::Hostname>
                        , remote_interface::Strategy
                        ) noexcept;

    //! \todo OPTIMIZE access to hostname via map<hostID, hostName>
  };
}

// IMPL

namespace gspc
{
  namespace remote_interface
  {
    namespace strategy
    {
      Thread::RemoteInterfaceServer::RemoteInterfaceServer (ID id)
        : _remote_interface (id)
        , _service_dispatcher()
        , _io_service (1)
        , _service_socket_provider (_io_service, _service_dispatcher)
        , _service_tcp_provider (_io_service, _service_dispatcher)
        , _local_endpoint ( fhg::util::connectable_to_address_string
                              (_service_tcp_provider.local_endpoint())
                          , _service_socket_provider.local_endpoint()
                          )
      {}
      rpc::endpoint Thread::RemoteInterfaceServer::endpoint() const
      {
        return _local_endpoint;
      }

      Thread::Thread (State* state)
        : _remote_interfaces_by_hostname {state}
      {}

      RuntimeSystemToRemoteInterface Thread::boot
        ( Hostname hostname
        , ID id
        ) const
      {
        auto const remote_interface
          (_remote_interfaces_by_hostname->emplace
               ( std::piecewise_construct
               , std::forward_as_tuple (hostname)
               , std::forward_as_tuple (id)
               )
          );

        if (!remote_interface.second)
        {
          throw std::invalid_argument
            (str ( boost::format ("strategy::Thread: Duplicate host '%1%'.")
                 % hostname
                 )
            );
        }

        return remote_interface.first->second.endpoint();
      }

      void Thread::teardown (Hostname hostname) const
      {
        if (!_remote_interfaces_by_hostname->erase (hostname))
        {
          throw std::invalid_argument
            (str ( boost::format ("strategy::Thread: Unknown host '%1%'.")
                 % hostname
                 )
            );
        }
      }
    }
  }
}

namespace gspc
{
  namespace remote_interface
  {
    ConnectionAndPID::ConnectionAndPID
      ( Hostname hostname
      , Strategy strategy
      , ID id
      )
        : _hostname {std::move (hostname)}
        , _strategy {std::move (strategy)}
        , _client { fhg::util::visit<RuntimeSystemToRemoteInterface>
                     ( _strategy
                     , [&] (auto const& s) { return s.boot (_hostname, id); }
                     )
                  }
    {}

    AnnotatedForest<Resource, MaybeError<resource::ID>>
      ConnectionAndPID::add (util::Forest<Resource> const& resources)
    {
      return _client.add (resources);
    }
  }
}

namespace gspc
{
  std::unordered_map
    < remote_interface::Hostname
    , MaybeError<remote_interface::ConnectionAndPID*>
    >
  RuntimeSystem::remote_interfaces
    ( std::unordered_set<remote_interface::Hostname> hostnames
    , remote_interface::Strategy strategy
    ) noexcept
  {
    std::unordered_map
      < remote_interface::Hostname
      , MaybeError<remote_interface::ConnectionAndPID*>
      > remote_interfaces;

    for (auto const& hostname : hostnames)
    {
      remote_interfaces[hostname] = bind
        ( [&]
          {
            auto remote_interface
              (_remote_interface_by_hostname.find (hostname));

            if (remote_interface == _remote_interface_by_hostname.end())
            {
              remote_interface = _remote_interface_by_hostname.emplace
                ( std::piecewise_construct
                , std::forward_as_tuple (hostname)
                , std::forward_as_tuple ( hostname
                                        , strategy
                                        , ++_next_remote_interface_id
                                        )
                ).first;
            }

            return &remote_interface->second;
          }
        );
    }

    return remote_interfaces;
  }

  std::unordered_map
    < remote_interface::Hostname
    , MaybeError<AnnotatedForest<Resource, MaybeError<resource::ID>>>
    >
    RuntimeSystem::add
      ( std::unordered_set<remote_interface::Hostname> hostnames
      , remote_interface::Strategy strategy
      , util::Forest<Resource> resources
      ) noexcept
  {
    std::unordered_map
      < remote_interface::Hostname
      , MaybeError<AnnotatedForest<Resource, MaybeError<resource::ID>>>
      > resources_by_host;

    for ( auto&& hostname_and_remote_interface
        : remote_interfaces (std::move (hostnames), std::move (strategy))
        )
    {
      resources_by_host.emplace
        ( std::move (hostname_and_remote_interface.first)
        , bind
          ( std::move (hostname_and_remote_interface.second)
          , [&] (remote_interface::ConnectionAndPID* connection)
            {
              return connection->add (resources);
            }
          )
        );
    }

    return resources_by_host;
  }

  std::unordered_set<resource::ID>
    RuntimeSystem::add_or_throw
      ( std::unordered_set<remote_interface::Hostname> hostnames
      , remote_interface::Strategy strategy
      , util::Forest<Resource> resources
      )
  {
    bool failed {false};
    std::unordered_set<resource::ID> resource_ids;

    for ( auto const& host_result
            : add ( hostnames
                  , std::move (strategy)
                  , std::move (resources)
                  ) | boost::adaptors::map_values
        )
    {
      if (is_failure (host_result))
      {
        failed = true;
      }
      else
      {
        for ( auto const& resource_result
            : boost::get<AnnotatedForest<Resource, MaybeError<resource::ID>>>
                (host_result)
            )
        {
          if (is_success (resource_result.second))
          {
            resource_ids.emplace
              (boost::get<resource::ID> (resource_result.second));
          }
          else
          {
            failed = true;
          }
        }
      }
    }

    if (failed)
    {
      remove (resource_ids);

      throw std::runtime_error ("failed to bootstrap");
    }

    return resource_ids;
  }
}

int main()
try
{
  gspc::resource_manager::Trivial resource_manager;

  gspc::RuntimeSystem runtime_system (resource_manager);

  gspc::remote_interface::strategy::Thread::State strategy_state;

  auto const resource_ids
    ( runtime_system.add_or_throw
      ( {"hostname"}
      , gspc::remote_interface::strategy::Thread {&strategy_state}
      , gspc::util::Forest<gspc::Resource> {}
      )
    );
  FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids); });

  gspc::PetriNetWorkflow workflow;
  gspc::PetriNetWorkflowEngine workflow_engine (workflow);

  gspc::GreedyScheduler scheduler
    ( workflow_engine
    , resource_manager
    , runtime_system
    );

  scheduler.wait();

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
