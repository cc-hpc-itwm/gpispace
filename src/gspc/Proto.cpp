#include <gspc/Proto.hpp>

namespace rpc
{
  std::unique_ptr<remote_endpoint> make_endpoint
    (boost::asio::io_service& io_service, endpoint ep)
  {
    return fhg::util::visit<std::unique_ptr<remote_endpoint>>
      ( ep.best (fhg::util::hostname())
      , [&] (socket_endpoint const& as_socket)
        {
          return std::make_unique<remote_socket_endpoint>
            (io_service, as_socket.socket);
        }
      , [&] (tcp_endpoint const& as_tcp)
        {
          return std::make_unique<remote_tcp_endpoint> (io_service, as_tcp);
        }
      );
  }
}

namespace gspc
{
  namespace remote_interface
  {
    bool operator== (ID const& lhs, ID const& rhs)
    {
      return std::tie (lhs.id) == std::tie (rhs.id);
    }
  }
  namespace resource
  {
    bool operator== (ID const& lhs, ID const& rhs)
    {
      return std::tie (lhs.remote_interface, lhs.id)
        == std::tie (rhs.remote_interface, rhs.id);
    }
  }

  bool operator== (Resource const& lhs, Resource const& rhs)
  {
    return std::tie (lhs._) == std::tie (rhs._);
  }

  RemoteInterface::RemoteInterface (remote_interface::ID id)
    : _next_resource_id {id}
    , _service_dispatcher()
    , _io_service (1)
    , _add ( _service_dispatcher
           , fhg::util::bind_this (this, &RemoteInterface::add)
           )
    , _service_socket_provider (_io_service, _service_dispatcher)
    , _service_tcp_provider (_io_service, _service_dispatcher)
    , _local_endpoint ( fhg::util::connectable_to_address_string
                          (_service_tcp_provider.local_endpoint())
                      , _service_socket_provider.local_endpoint()
                      )
  {}

  rpc::endpoint const& RemoteInterface::local_endpoint() const
  {
    return _local_endpoint;
  }

  util::AnnotatedForest<Resource, ErrorOr<resource::ID>>
    RemoteInterface::add (util::Forest<Resource> const& resources)
  {
    return resources.combining_transform
      ( [&] ( Resource const& resource
            , std::list<ErrorOr<resource::ID> const*> const& children
            )
        {
          if (std::any_of ( children.cbegin()
                          , children.cend()
                          , [] (auto const child)
                            {
                              return !*child;
                            }
                          )
             )
          {
            throw std::runtime_error
              (str ( boost::format ("Skip start of '%1%': Child failure.")
                   % resource._
                   )
              );
          }

          // start resource: fork process

          return ++_next_resource_id;
        }
      );
  }

  namespace remote_interface
  {
    namespace strategy
    {
      Thread::RemoteInterfaceServer::RemoteInterfaceServer (ID id)
        : _remote_interface (id)
      {}
      rpc::endpoint Thread::RemoteInterfaceServer::local_endpoint() const
      {
        return _remote_interface.local_endpoint();
      }

      Thread::Thread (State* state)
        : _remote_interfaces_by_hostname {state}
      {}

      rpc::endpoint Thread::boot
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

        return remote_interface.first->second.local_endpoint();
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
    RuntimeSystemToRemoteInterface::RuntimeSystemToRemoteInterface
        (boost::asio::io_service& io_service, rpc::endpoint endpoint)
      : _endpoint {rpc::make_endpoint (io_service, endpoint)}
      , add {*_endpoint}
    {}

    ConnectionAndPID::ConnectionAndPID
      ( boost::asio::io_service& io_service
      , Hostname hostname
      , Strategy strategy
      , ID id
      )
        : _hostname {std::move (hostname)}
        , _strategy {std::move (strategy)}
        , _client { io_service
                  , fhg::util::visit<rpc::endpoint>
                      ( _strategy
                      , [&] (auto const& s) { return s.boot (_hostname, id); }
                      )
                  }
    {}
    ConnectionAndPID::~ConnectionAndPID()
    {
      fhg::util::visit<void>
        ( _strategy
        , [&] (auto const& s) { return s.teardown (_hostname); }
        );
    }

    util::AnnotatedForest<Resource, ErrorOr<resource::ID>>
      ConnectionAndPID::add (util::Forest<Resource> const& resources)
    {
      return _client.add (resources).get();
    }
  }
}

namespace gspc
{
  std::unordered_map
    < remote_interface::Hostname
    , ErrorOr<remote_interface::ConnectionAndPID*>
    >
  ScopedRuntimeSystem::remote_interfaces
    ( std::unordered_set<remote_interface::Hostname> hostnames
    , remote_interface::Strategy strategy
    ) noexcept
  {
    std::unordered_map
      < remote_interface::Hostname
      , ErrorOr<remote_interface::ConnectionAndPID*>
      > remote_interfaces;

    for (auto const& hostname : hostnames)
    {
      remote_interfaces.emplace
        ( hostname
        , [&]
          {
            auto remote_interface
              (_remote_interface_by_hostname.find (hostname));

            if (remote_interface == _remote_interface_by_hostname.end())
            {
              remote_interface = _remote_interface_by_hostname.emplace
                ( std::piecewise_construct
                , std::forward_as_tuple (hostname)
                , std::forward_as_tuple ( _remote_interface_io_service
                                        , hostname
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
    , ErrorOr<util::AnnotatedForest<Resource, ErrorOr<resource::ID>>>
    >
    ScopedRuntimeSystem::add
      ( std::unordered_set<remote_interface::Hostname> hostnames
      , remote_interface::Strategy strategy
      , util::Forest<Resource> resources
      ) noexcept
  {
    //! \note syntax goal
    // return remote_interfaces (std::move (hostnames), std::move (strategy))
    //   >> [&] (remote_interface::ConnectionAndPID* connection)
    //         {
    //           return connection->add (resources);
    //         }

    std::unordered_map
      < remote_interface::Hostname
      , ErrorOr<util::AnnotatedForest<Resource, ErrorOr<resource::ID>>>
      > resources_by_host;

    for ( auto&& hostname_and_remote_interface
        : remote_interfaces (std::move (hostnames), std::move (strategy))
        )
    {
      resources_by_host.emplace
        ( std::move (hostname_and_remote_interface.first)
        , std::move (hostname_and_remote_interface.second)
          >> [&] (remote_interface::ConnectionAndPID* connection)
             {
               //! \todo if adding the resources fails, we do *not*
               //! stop the rif again. is this bad? is this good
               //! because 99% there will be a retry anyway? what is
               //! our post-condition? (note: we do not leak it, we
               //! remember we started one.)

               //! \note to try-catch and remove rif on error is _not_
               //! enough: the rif might have been started in a
               //! previous call
               return connection->add (resources);
             }
        );
    }

    return resources_by_host;
  }

  std::unordered_set<resource::ID>
    ScopedRuntimeSystem::add_or_throw
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
      if (!host_result)
      {
        failed = true;
      }
      else
      {
        for (auto const& resource_result : host_result.value())
        {
          if (resource_result.second)
          {
            resource_ids.emplace (resource_result.second.value());
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

  gspc::ScopedRuntimeSystem runtime_system (resource_manager);

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
