#include <gspc/Proto.hpp>

#include <util-generic/nest_exceptions.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/lexical_cast.hpp>

#include <exception>
#include <vector>

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
    std::ostream& operator<< (std::ostream& os, ID const& x)
    {
      return os << "remote_interface " << x.id;
    }
  }
  namespace resource
  {
    bool operator== (ID const& lhs, ID const& rhs)
    {
      return std::tie (lhs.remote_interface, lhs.id)
        == std::tie (rhs.remote_interface, rhs.id);
    }
    std::ostream& operator<< (std::ostream& os, ID const& x)
    {
      return os << x.remote_interface << " resource " << x.id;
    }
  }
  namespace task
  {
    bool operator== (ID const& lhs, ID const& rhs)
    {
      return std::tie (lhs.id) == std::tie (rhs.id);
    }
  }

  bool operator== (Resource const& lhs, Resource const& rhs)
  {
    return std::tie (lhs.resource_class) == std::tie (rhs.resource_class);
  }
  std::ostream& operator<< (std::ostream& os, Resource const& r)
  {
    return os << "resource " << r.resource_class;
  }

  namespace comm
  {
    namespace scheduler
    {
      namespace worker
      {
        Client::Client
            (boost::asio::io_service& io_service, rpc::endpoint endpoint)
          : _endpoint {rpc::make_endpoint (io_service, std::move (endpoint))}
          , submit {*_endpoint}
          , cancel {*_endpoint}
        {}

        template<typename Submit, typename Cancel>
            Server::Server (Submit&& submit, Cancel&& cancel)
          : _service_dispatcher()
          , _io_service (1)
          , _submit (_service_dispatcher, std::forward<Submit> (submit))
          , _cancel (_service_dispatcher, std::forward<Cancel> (cancel))
          , _service_socket_provider (_io_service, _service_dispatcher)
          , _service_tcp_provider (_io_service, _service_dispatcher)
          , _local_endpoint ( fhg::util::connectable_to_address_string
                                (_service_tcp_provider.local_endpoint())
                            , _service_socket_provider.local_endpoint()
                            )
        {}

        template<typename That>
          Server::Server (That* that)
            : Server ( fhg::util::bind_this (that, &That::submit)
                     , fhg::util::bind_this (that, &That::cancel)
                     )
        {}

        rpc::endpoint Server::local_endpoint() const
        {
          return _local_endpoint;
        }
      }
    }
  }

  namespace comm
  {
    namespace worker
    {
      namespace scheduler
      {
        Client::Client
            (boost::asio::io_service& io_service, rpc::endpoint endpoint)
          : _endpoint {rpc::make_endpoint (io_service, std::move (endpoint))}
          , finished {*_endpoint}
        {}

        template<typename Finished>
            Server::Server (Finished&& submit)
          : _service_dispatcher()
          , _io_service (1)
          , _finished (_service_dispatcher, std::forward<Finished> (submit))
          , _service_socket_provider (_io_service, _service_dispatcher)
          , _service_tcp_provider (_io_service, _service_dispatcher)
          , _local_endpoint ( fhg::util::connectable_to_address_string
                                (_service_tcp_provider.local_endpoint())
                            , _service_socket_provider.local_endpoint()
                            )
        {}

        template<typename That>
          Server::Server (That* that)
            : Server (fhg::util::bind_this (that, &That::finished))
        {}

        rpc::endpoint Server::local_endpoint() const
        {
          return _local_endpoint;
        }
      }
    }
  }

  Worker::Worker (Resource resource)
    : _resource {std::move (resource)}
    , _comm_server_for_scheduler (this)
  {}

  rpc::endpoint Worker::endpoint_for_scheduler() const
  {
    return _comm_server_for_scheduler.local_endpoint();
  }

  void Worker::submit (rpc::endpoint scheduler, job::ID job_id, Job job)
  {
    auto const& task (job.task);

    if (task.so == "map_so" && task.symbol == "map_symbol")
    {
      auto const& inputs (task.inputs);

      if (  inputs.size() != 3
         || inputs.count ("input") != 1
         || inputs.count ("output") != 1
         || inputs.count ("N") != 1
         || inputs.at ("input") != task.id.id
         || inputs.at ("input") + inputs.at ("output") != inputs.at ("N")
         )
      {
        throw std::logic_error ("Worker::submit: Corrupted task.");
      }

      comm::worker::scheduler::Client (_io_service_for_scheduler, scheduler)
        .finished (job_id, job::finish_reason::Success ({inputs}));

      return;
    }

    throw std::invalid_argument ("Worker::submit: non-Map: NYI.");
  }
  void Worker::cancel (job::ID)
  {
    throw std::runtime_error ("Worker::cancel: NYI");
  }


  RemoteInterface::RemoteInterface (remote_interface::ID id)
    : _next_resource_id {id}
    , _service_dispatcher()
    , _io_service (1)
      //! BEGIN Syntax goal
      //! _comm_server ( fhg::util::bind_this (this, &RemoteInterface::add)
      //!              , fhg::util::bind_this (this, &RemoteInterface::remove)
      //!              , fhg::util::bind_this
      //!                  (this, &RemoteInterface::worker_endpoint_for_scheduler)
      //!              )
      //! Even better:
      //! _comm_server (this)
    , _add ( _service_dispatcher
           , fhg::util::bind_this (this, &RemoteInterface::add)
           )
    , _remove ( _service_dispatcher
              , fhg::util::bind_this (this, &RemoteInterface::remove)
              )
    , _worker_endpoint_for_scheduler
        ( _service_dispatcher
        , fhg::util::bind_this
            (this, &RemoteInterface::worker_endpoint_for_scheduler)
        )
      //! END Syntax goal
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

  rpc::endpoint RemoteInterface::worker_endpoint_for_scheduler
    (resource::ID resource_id) const
  {
    auto worker (_workers.find (resource_id));
    if (worker == _workers.end())
    {
      throw std::invalid_argument
        ("worker_endpoint_for_scheduler (unknown resource)");
    }
    return worker->second.endpoint_for_scheduler();
  }

  RemoteInterface::WorkerServer::WorkerServer (Resource const& resource)
    : _worker (resource)
  {}
  rpc::endpoint RemoteInterface::WorkerServer::endpoint_for_scheduler() const
  {
    return _worker.endpoint_for_scheduler();
  }

  UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
    RemoteInterface::add (UniqueForest<Resource> const& resources)
  {
    using ResultNode
      = unique_forest::Node<std::tuple<Resource, ErrorOr<resource::ID>>>;

    return resources.upward_combine_transform
      ( [&] ( unique_forest::Node<Resource> const& resource
            , std::list<ResultNode const*> const& children
            ) -> ResultNode
        {
          return
            { resource.first
            , std::tuple<Resource, ErrorOr<resource::ID>>
              ( resource.second
              , [&]
                {
                  if (std::any_of ( children.cbegin()
                                  , children.cend()
                                  , [] (auto const& child)
                                    {
                                      return !std::get<1> (child->second);
                                    }
                                  )
                     )
                  {
                    throw std::runtime_error
                      (str ( boost::format ("Skip start of '%1%': Child failure.")
                           % resource.second
                           )
                      );
                  }

                  return _workers.emplace ( ++_next_resource_id
                                          , resource.second
                                          ).first->first;
                }
              )
            };
        }
      );
  }

  Forest<resource::ID, ErrorOr<>>
    RemoteInterface::remove (Forest<resource::ID> const& resources)
  {
    using ResultNode = forest::Node<resource::ID, ErrorOr<>>;

    return resources.unordered_transform
      ( [&] (forest::Node<resource::ID> const& id) -> ResultNode
        {
          return
            { id.first
            , [&] () -> boost::blank
              {
                if (!_workers.erase (id.first))
                {
                  throw std::invalid_argument
                    (str ( boost::format
                         ("RemoteInterface::remove: Unknown worker '%1%'.")
                         % id.first
                         )
                    );
                }

                return {};
              }
            };
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
          (_remote_interfaces_by_hostname->emplace (hostname, id));

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
  namespace comm
  {
    namespace runtime_system
    {
      namespace remote_interface
      {
        Client::Client
          (boost::asio::io_service& io_service, rpc::endpoint endpoint)
            : _endpoint {rpc::make_endpoint (io_service, std::move (endpoint))}
            , add {*_endpoint}
            , remove {*_endpoint}
            , worker_endpoint_for_scheduler {*_endpoint}
        {}
      }
    }
  }
}

namespace gspc
{
  namespace remote_interface
  {
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
    Strategy const& ConnectionAndPID::strategy() const
    {
      return _strategy;
    }
    ConnectionAndPID::~ConnectionAndPID()
    {
      fhg::util::visit<void>
        ( _strategy
        , [&] (auto const& s) { return s.teardown (_hostname); }
        );
    }

    UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
      ConnectionAndPID::add (UniqueForest<Resource> const& resources)
    {
      return _client.add (resources);
    }

    Forest<resource::ID, ErrorOr<>>
      ConnectionAndPID::remove (Forest<resource::ID> const& resources)
    {
      return _client.remove (resources);
    }

    rpc::endpoint ConnectionAndPID::worker_endpoint_for_scheduler
      (resource::ID resource_id)
    {
      return _client.worker_endpoint_for_scheduler (resource_id);
    }
  }
}

namespace gspc
{
  ScopedRuntimeSystem::ScopedRuntimeSystem
      (comm::runtime_system::resource_manager::Client resource_manager)
    : _resource_manager (resource_manager)
  {}

  std::unordered_map
    < remote_interface::Hostname
    , ErrorOr<remote_interface::ConnectionAndPID*>
    >
  ScopedRuntimeSystem::remote_interfaces
    ( std::unordered_set<remote_interface::Hostname> hostnames
    , remote_interface::Strategy strategy
    ) noexcept
  {
    // syntax goal:
    // return hostnames
    //   >>  [&] ...

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
              auto const id (++_next_remote_interface_id);

              remote_interface = _remote_interface_by_hostname.emplace
                ( hostname
                , std::make_unique<remote_interface::ConnectionAndPID>
                    ( _remote_interface_io_service
                    , hostname
                    , strategy
                    , id
                    )
                ).first;

              _hostname_by_remote_interface_id.emplace (id, hostname);
            }
            //! \todo specify: would it be okay to use a second
            //! strategy for the same host?
            // else
            // {
            //   if (remote_interface->second.strategy() != strategy)
            //   {
            //     //! \todo more information in exception!?
            //     throw std::invalid_argument ("Different strategy");
            //   }
            // }

            return remote_interface->second.get();
          }
        );
    }

    return remote_interfaces;
  }

  remote_interface::ConnectionAndPID*
    ScopedRuntimeSystem::remote_interface_by_id
      (remote_interface::ID remote_interface_id) const
  try
  {
    return _remote_interface_by_hostname
      .at (_hostname_by_remote_interface_id.at (remote_interface_id)).get()
      ;
  }
  catch (...)
  {
    std::throw_with_nested
      ( std::invalid_argument
          ( "ScopedRuntimeSystem::remote_interface_by_id: "
            "Unknown remote_interface_id"
          )
      );
  }

  std::unordered_map
    < remote_interface::Hostname
    , ErrorOr<UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>>
    >
    ScopedRuntimeSystem::add
      ( std::unordered_set<remote_interface::Hostname> hostnames
      , remote_interface::Strategy strategy
      , UniqueForest<Resource> const& resources
      ) noexcept
  {
    return remote_interfaces (std::move (hostnames), std::move (strategy))
      >> [&] ( remote_interface::Hostname const&
             , remote_interface::ConnectionAndPID* connection
             )
         {
           //! \todo if adding the resources fails, we do *not* stop
           //! the rif again. is this bad? is this good because 99%
           //! there will be a retry anyway? what is our
           //! post-condition? (note: we do not leak it, we remember
           //! we started one.)

           //! \note to try-catch and remove rif on error is _not_
           //! enough: the rif might have been started in a previous
           //! call

           //! \note if the post condition of connection->add
           //! (resources) is "all or nothing", then it would be
           //! enough to remove the rif if there are not other
           //! resources on the same rif (in this case the rif _was_
           //! started by this incarnation)
           return connection->add (resources);
         }
      >> [&] ( remote_interface::Hostname const&
             , UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>> result
             )
         {
           using Node
             = unique_forest::Node<std::tuple<Resource, ErrorOr<resource::ID>>>;

           //! split (result).first.unordered_transform () -> Resources::Node
           _resource_manager.add
             ( UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
                 (result)
             . remove_root_if
                 ([] (Node const& node) { return !std::get<1> (node.second); })
             . unordered_transform
                 ( [] (Node const& node)
                   {
                     return interface::ResourceManager::Resources::Node
                       ( std::get<1> (node.second).value()
                       , std::get<0> (node.second).resource_class
                       );
                   }
                 )
             );

           return result;
         };
      //! \todo
      //      |= [&] ( remote_interface::Hostname const& // hostname
      //             , MaybeError<UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>> // result
      //             )
      //         {
      //           // teardown rif if empty
      //         };
  }

  Forest<resource::ID, ErrorOr<>>
    ScopedRuntimeSystem::remove (Forest<resource::ID> const& to_remove)
  {
    //! \note checks connected components do not cross
    //! remote_interface boundaries
    std::unordered_map < remote_interface::ID
                       , Forest<resource::ID>
                       > const to_remove_by_host
      ( to_remove.multiway_split
        ( [&] (forest::Node<resource::ID> const& resource_id)
          {
            return resource_id.first.remote_interface;
          }
        )
      );

    //! \note checks resource is known (before doing anything)
    _resource_manager.remove (to_remove);

    using Results = Forest<resource::ID, ErrorOr<>>;

    Results results;

    std::for_each
      ( to_remove_by_host.begin(), to_remove_by_host.end()
      , [&] (auto const& rif_and_to_remove)
        {
          //! SAFE: each forest was part of `to_remove`
          results.UNSAFE_merge
            ( [&]
              {
                try
                {
                  return remote_interface_by_id (rif_and_to_remove.first)
                    ->remove (rif_and_to_remove.second);
                }
                catch (...)
                {
                  return rif_and_to_remove.second.unordered_transform
                    ( [&] (forest::Node<resource::ID> const& resource_id)
                      {
                        return Results::Node
                          {resource_id.first, std::current_exception()};
                      }
                    );
                }
              }()
            );
        }
      );

    return results;
  }

  rpc::endpoint ScopedRuntimeSystem::worker_endpoint_for_scheduler
    (resource::ID resource_id) const
  {
    return remote_interface_by_id (resource_id.remote_interface)
      ->worker_endpoint_for_scheduler (resource_id);
  }

  namespace
  {
    std::exception_ptr nest_with_runtime_error
      (std::exception_ptr ex, std::string what)
    {
      try
      {
        fhg::util::nest_exceptions<std::runtime_error>
          ([&] { std::rethrow_exception (ex); }, what);
      }
      catch (...)
      {
        return std::current_exception();
      }

      throw std::logic_error ("null exception pointer");
    }
  }

  //! \todo think about combinators like this
  template<typename T, typename U>
    std::pair<Forest<T, U>, Forest<T, std::exception_ptr>>
      split (Forest<T, ErrorOr<U>> forest)
  {
    Forest<T, ErrorOr<U>> copy (forest);

    return
      { std::move (forest)
      . remove_root_if
        ( [] (forest::Node<T, ErrorOr<U>> const& node)
          {
            return !node.second;
          }
        )
      . unordered_transform
        ( [] (forest::Node<T, ErrorOr<U>> const& node)
          {
            return forest::Node<T, U> (node.first, node.second.value());
          }
       )
      , std::move (copy)
      . remove_root_if
        ( [] (forest::Node<T, ErrorOr<U>> const& node)
          {
            return node.second;
          }
        )
      . unordered_transform
        ( [] (forest::Node<T, ErrorOr<U>> const& node)
          {
            return forest::Node<T, std::exception_ptr>
              (node.first, node.second.error());
          }
       )
      };
  }

  Forest<resource::ID>
    ScopedRuntimeSystem::add_or_throw
      ( std::unordered_set<remote_interface::Hostname> hostnames
      , remote_interface::Strategy strategy
      , UniqueForest<Resource> const& resources
      )
  {
    std::vector<std::exception_ptr> failures;
    Forest<resource::ID> successes;

    for ( auto const& host_result
        : add (hostnames, std::move (strategy), resources)
        )
    {
      if (!host_result.second)
      {
        failures.emplace_back
          ( nest_with_runtime_error
              (host_result.second.error(), "failure on " + host_result.first)
          );
      }
      else
      {
        //! SAFE: each host includes its (unique) remote_interface_id
        successes.UNSAFE_merge
          ( host_result.second.value()
          . unordered_transform
              ( [&] (unique_forest::Node<std::tuple<Resource, ErrorOr<resource::ID>>> const& node)
                {
                  if (!std::get<1> (node.second))
                  {
                    failures.emplace_back
                      ( nest_with_runtime_error
                          ( std::get<1> (node.second).error()
                          , "failure on " + host_result.first
                          + " for resource "
                          + boost::lexical_cast<std::string>
                              (std::get<0> (node.second))
                          )
                      );
                  }

                  return unique_forest::Node<ErrorOr<resource::ID>>
                    (node.first, std::get<1> (node.second));
                }
              )
          . remove_root_if
              ( [] (unique_forest::Node<ErrorOr<resource::ID>> const& node)
                {
                  return !node.second;
                }
              )
          . unordered_transform
              ( [] (unique_forest::Node<ErrorOr<resource::ID>> const& node)
                {
                  return forest::Node<resource::ID> (node.second.value(), {});
                }
              )
          );
      }
    }

    if (!failures.empty())
    {
      remove (successes).for_each_node
        ( [&] (forest::Node<resource::ID, ErrorOr<>> const& remove_result)
          {
            if (!remove_result.second)
            {
              failures.emplace_back
                ( nest_with_runtime_error
                  ( remove_result.second.error()
                  , "when trying to clean up successfully started resources"
                  )
                );
            }
          }
        );

      fhg::util::nest_exceptions<std::runtime_error>
        ( [&] { fhg::util::throw_collected_exceptions (failures); }
        , "adding resources failures"
        );
    }

    return successes;
  }
}

namespace gspc
{
  namespace resource_manager
  {
    void WithPreferences::interrupt()
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      _interrupted = true;

      _resources_became_available_or_interrupted.notify_all();
    }

    void WithPreferences::add (Resources new_resources)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      //! \note already asserts precondition of uniqueness
      _resources.merge (new_resources);

      new_resources.for_each_node
        ( [&] (Resources::Node const& resource)
          {
            _resource_usage_by_id.emplace (resource.first, 0);
            _available_resources_by_class[resource.second]
              . emplace (resource.first)
              ;
          }
        );

      _resources_became_available_or_interrupted.notify_all();
    }

    void WithPreferences::remove (Forest<resource::ID> to_remove)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      to_remove.for_each_node
        ( [&] (forest::Node<resource::ID> const& resource)
          {
            if (!_resource_usage_by_id.count (resource.first))
            {
              throw std::invalid_argument ("Unknown.");
            }
            if (_resource_usage_by_id.at (resource.first))
            {
              throw std::invalid_argument ("In use.");
            }
          }
        );

      to_remove.upward_apply
        ( [&] (forest::Node<resource::ID> const& resource)
          {
            auto const removed_node (_resources.remove_leaf (resource.first));
            _resource_usage_by_id.erase (removed_node.first);
            _available_resources_by_class.at (removed_node.second)
              . erase (removed_node.first)
              ;
          }
        );
    }

    namespace
    {
      template<typename Collection>
        auto select_any (Collection& collection)
          -> decltype (*collection.begin())
      {
        return *collection.begin();
      }
    }

    WithPreferences::Acquired WithPreferences::acquire
      (std::list<resource::Class> resource_classes)
    {
      std::unique_lock<std::mutex> resources_lock (_resources_guard);

      auto const has_available_resource
        ( [&] (resource::Class resource_class)
          {
            return _available_resources_by_class.count (resource_class)
              && !_available_resources_by_class.at (resource_class).empty()
              ;
          }
        );

      _resources_became_available_or_interrupted.wait
        ( resources_lock
        , [&]
          {
            return std::any_of ( resource_classes.begin()
                               , resource_classes.end()
                               , has_available_resource
                               )
              || _interrupted
              ;
          }
        );

      if (_interrupted)
      {
        throw Interrupted{};
      }

      auto const resource_class
        ( *std::find_if ( resource_classes.begin()
                        , resource_classes.end()
                        , has_available_resource
                        )
        );

      auto const requested
        (select_any (_available_resources_by_class.at (resource_class)));

      //! block every resource `child*` that is down-reachable because
      //! `requested` is a (transitive) parent of `child*`

      //! block every (transitive) parent `(other_)parent*` of all
      //! down-reachable resources `child*` because one of their child
      //! has been blocked

      //! parent -> requested -> child1 -> child11
      //!        -> sibling~            -> child12
      //!                     -> child2 <- other_parent2
      //!                     -> child3 <- other_parent31 <- other_parent311
      //!                               <- other_parent32
      //!               other_child321~ <-

      //! Note: sibling~ and other_child321~ are _not_ blocked

      //! In this forest when requested is blocked, then all resources
      //! marked by * or # are blocked, too. The numbers after the *
      //! are the changes in the values of the reference counters, the
      //! number after the # are the exact values of the reference
      //! counters (they must have been zero before the acquisition)
      //!
      //!       A*4 --+       G*4 ..>       J*1
      //!       |     |       |             |
      //!       v     |       v             v
      //!       B     +-> requested#4 --+   H*1 --> I
      //!       |             |         |   |
      //!       v             v         |   v
      //!       F             C#2       +-> E#1
      //!                     |
      //!                     v
      //!                     D#1

      //! Note:
      //! - G ..> means that G might have more children. If not, then * is #.
      //! - B and F are not blocked as it would require going down
      //! from A again.
      //! - While blocking E blocks it's parent H, again, I is not
      //! blocked due to up-down required.
      //! - To release anything but `requested` will lead to chaos!

      _resources.down_up
        ( requested
        , [&] (Resources::Node const& x)
          {
            if (0 != ++_resource_usage_by_id.at (x.first))
            {
              _available_resources_by_class.at (x.second).erase (x.first);
            }
          }
        );

      return Acquired {requested /*, up-visited*/};
    }

    void WithPreferences::release (Acquired const& to_release)
    {
      return release (to_release.requested);
    }
    void WithPreferences::release (resource::ID const& to_release)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      _resources.down_up
        ( to_release
        , [&] (Resources::Node const& x)
          {
            if (0 == --_resource_usage_by_id.at (x.first))
            {
              _available_resources_by_class.at (x.second).emplace (x.first);
            }
          }
        );

      _resources_became_available_or_interrupted.notify_all();
    }

    void Coallocation::assert_is_strictly_forward_disjoint_by_resource_class
      (Resources const& resources)
    {
      resources.for_each_leaf
        ( [&] (Resources::Node const& start)
          {
            std::unordered_map<resource::Class, std::size_t>
              visible_resources_by_class;

            resources.up
              ( start.first
              , [&] (Resources::Node const& visible)
                {
                  if (visible_resources_by_class[visible.second]++)
                  {
                    throw std::logic_error
                      ("not strictly_forward_disjoint_by_resource_class");
                  }
                }
              );
          }
        );
    }

    void Coallocation::interrupt()
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      _interrupted = true;

      _resources_became_available_or_interrupted.notify_all();
    }

    void Coallocation::add (Resources new_resources)
    {
      //! \note We can't add connections to existing resources, so
      //! checking addition is enough.
      assert_is_strictly_forward_disjoint_by_resource_class (new_resources);

      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      //! \note already asserts precondition of uniqueness
      _resources.merge (new_resources);

      new_resources.for_each_node
        ( [&] (Resources::Node const& resource)
          {
            _resource_usage_by_id.emplace (resource.first, 0);
            _available_resources_by_class[resource.second]
              . emplace (resource.first)
              ;
          }
        );

      _resources_became_available_or_interrupted.notify_all();
    }

    void Coallocation::remove (Forest<resource::ID> to_remove)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      to_remove.for_each_node
        ( [&] (forest::Node<resource::ID> const& resource)
          {
            if (!_resource_usage_by_id.count (resource.first))
            {
              throw std::invalid_argument ("Unknown.");
            }
            if (_resource_usage_by_id.at (resource.first))
            {
              throw std::invalid_argument ("In use.");
            }
          }
        );

      //! required: upwards or downwards per unconnected sub-tree,
      //! just *not random*. Every block that happened before removal
      //! has to still block during (or have one side of the block
      //! vanished).
      to_remove.upward_apply
        ( [&] (forest::Node<resource::ID> const& resource)
          {
            auto const removed_node (_resources.remove_leaf (resource.first));
            _resource_usage_by_id.erase (removed_node.first);
            _available_resources_by_class.at (removed_node.second)
              . erase (removed_node.first)
              ;
          }
        );
    }

    namespace
    {
      template<typename ResourceIDs>
        std::unordered_set<resource::ID> select_any
          (ResourceIDs& collection, std::size_t count)
      {
        auto begin (collection.begin());
        return {begin, std::next (begin, count)};
      }
    }

    Coallocation::Acquired Coallocation::acquire
      (resource::Class resource_class, std::size_t count)
    {
      std::unique_lock<std::mutex> resources_lock (_resources_guard);

      auto const has_available_resources
        ( [&] (resource::Class resource_class)
          {
            return _available_resources_by_class.count (resource_class)
              && _available_resources_by_class.at (resource_class).size()
                 >= count
              ;
          }
        );

      _resources_became_available_or_interrupted.wait
        ( resources_lock
        , [&]
          {
            return has_available_resources (resource_class) || _interrupted;
          }
        );

      if (_interrupted)
      {
        throw Interrupted{};
      }

      auto const requesteds
        (select_any (_available_resources_by_class.at (resource_class), count));

      //! \note Forward-disjoint is required here: All traversals are
      //! downwards independent (they may reach the same resources
      //! upwards though), but one element of requesteds will never
      //! lock another one.
      std::for_each
        ( requesteds.begin()
        , requesteds.end()
        , [&] (auto const& requested)
          {
            _resources.down_up
              ( requested
              , [&] (Resources::Node const& x)
                {
                  if (0 != ++_resource_usage_by_id.at (x.first))
                  {
                    _available_resources_by_class.at (x.second).erase (x.first);
                  }
                }
              );
          }
        );

      return Acquired {requesteds /*, up-visited*/};
    }

    void Coallocation::release (Acquired const& to_release)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      std::for_each ( to_release.requesteds.begin()
                    , to_release.requesteds.end()
                    , [&] (resource::ID const& id)
                      {
                        release (id);
                      }
                    );

      _resources_became_available_or_interrupted.notify_all();
    }
    void Coallocation::release (resource::ID const& to_release)
    {
      _resources.down_up
        ( to_release
        , [&] (Resources::Node const& x)
          {
            if (0 == --_resource_usage_by_id.at (x.first))
            {
              _available_resources_by_class.at (x.second).emplace (x.first);
            }
          }
        );
    }
  }
}

namespace gspc
{
  namespace interface
  {
    template<typename Derived>
        Scheduler::Scheduler (Derived* derived)
      : _comm_server_for_worker (derived)
    {}
  }
}

namespace gspc
{
  GreedyScheduler::GreedyScheduler
      ( comm::scheduler::workflow_engine::Client workflow_engine
      , resource_manager::Trivial& resource_manager
      , ScopedRuntimeSystem& runtime_system
      )
        //! \todo Is it okay to construct the server before
        //! constructing the state?!
    : Scheduler (this)
    , _workflow_engine (workflow_engine)
    , _resource_manager (resource_manager)
    , _runtime_system (runtime_system)
    , _thread (fhg::util::bind_this ( this
                                    , &GreedyScheduler::scheduling_thread
                                    )
              )
  {}

  template<typename Lock, typename Fun, typename... Args>
    auto call_unlocked (Lock& lock, Fun&& fun, Args&&... args)
      -> decltype (std::forward<Fun> (fun) (std::forward<Args> (args)...))
  {
    FHG_UTIL_FINALLY ([&] { lock.lock(); });
    lock.unlock();
    return std::forward<Fun> (fun) (std::forward<Args> (args)...);
  }

  void GreedyScheduler::scheduling_thread()
  {
    std::unique_lock<std::mutex> lock (_guard_state);

    while (!_stopped)
    {
      fhg::util::visit<void>
        ( _workflow_engine.extract()
        , [&] (Task const& task)
          {
            try
            {
              auto const acquired
                ( call_unlocked
                    ( lock
                    , [&]
                      {
                        return _resource_manager.acquire (task.resource_class);
                      }
                    )
                );

              if (!_tasks.emplace (task.id, acquired.requested).second)
              {
                throw std::logic_error ("INCONSISTENCY: Duplicate task id.");
              }

              call_unlocked
                ( lock
                , [&]
                  {
                    try
                    {
                      auto const worker_endpoint
                        ( _runtime_system.worker_endpoint_for_scheduler
                            (acquired.requested)
                        );
                      //! \todo job_id
                      comm::scheduler::worker::Client ( _io_service_for_workers
                                                      , worker_endpoint
                                                      )
                        . submit ( _comm_server_for_worker.local_endpoint()
                                 , job::ID {0, task.id}
                                 , Job {task}
                                 );
                    }
                    catch (...)
                    {
                      finished ( {0, task.id}
                               , job::finish_reason::WorkerFailure
                                   {std::current_exception()}
                               );
                    }
                  }
                );
            }
            catch (interface::ResourceManager::Interrupted const&)
            {
              assert (_stopped);
            }
          }
        , [&] (bool has_finished)
          {
            if (!has_finished && !_tasks.empty())
            {
              _injected_or_stopped
                .wait (lock, [&] { return _injected || !!_stopped; });

              if (_injected)
              {
                _injected = false;
              }
            }
            else if (!has_finished && _tasks.empty())
            {
              // \todo wait for external event (put_token), go again
              // into extract if not stopped
              _injected_or_stopped
                .wait ( lock
                      , [&] { return /* _put_token || */ !!_stopped; }
                      );

              // if (_put_token)
              // {
              //   _put_token = false;
              // }
            }
            else if (has_finished && _tasks.empty())
            {
              _stopped = true;
            }
            else // if (has_finished && !_tasks.empty())
            {
              throw std::logic_error
                ("INCONSISTENCY: finished while tasks are running.");
            }
          }
        );
    }

    //! \todo
    // for (auto const& task : _tasks)
    // {
    //   _runtime_system.cancel (task);
    // }

    _injected_or_stopped.wait (lock, [&] { return _tasks.empty(); });
  }

  void GreedyScheduler::finished
    (job::ID job_id, job::FinishReason finish_reason)
  {
    auto const task_id (job_id.task_id);

    std::lock_guard<std::mutex> const lock (_guard_state);

    auto remove_task
      ( [&]
        {
          _resource_manager.release
            (resource_manager::Trivial::Acquired {_tasks.at (task_id)});

          if (!_tasks.erase (task_id))
          {
            throw std::logic_error ("INCONSISTENCY: finished unknown tasks");
          }
        }
      );

    fhg::util::visit<void>
      ( finish_reason
      , [&] (job::finish_reason::Success const& success)
        {
          _workflow_engine.inject (task_id, success.result.task_result);

          remove_task();

          _injected = true;

          _injected_or_stopped.notify_one();
        }
      , [&] (job::finish_reason::JobFailure const&)
        {
          remove_task();

          stop();
        }
      , [] (job::finish_reason::WorkerFailure const&)
        {
          //! \todo re-schedule? Beware: May be _stopped already!
          throw std::logic_error ("NYI: finished (WorkerFailure)");
        }
      , [&] (job::finish_reason::Cancelled const&)
        {
          //! \todo sanity!?
          //! do nothing, just remove task
          remove_task();
        }
      );
  }

  void GreedyScheduler::wait()
  {
    if (_thread.joinable())
    {
      _thread.join();
    }
  }

  void GreedyScheduler::stop()
  {
    _stopped = true;

    _resource_manager.interrupt();

    _injected_or_stopped.notify_one();
  }
}

namespace gspc
{
  MapWorkflowEngine::MapWorkflowEngine (std::uint64_t N)
    : _N (N)
  {}

  boost::variant<Task, bool> MapWorkflowEngine::extract()
  {
    if (_i++ < _N)
    {
      return Task { "core"
                  , {*_extracted.emplace (++_next_task_id).first}
                  , {{"input", _i}, {"output", _N - _i}, {"N", _N}}
                  , {"map_so"}
                  , {"map_symbol"}
                  };
    }
    else
    {
      return _extracted.empty();
    }
  }

  void MapWorkflowEngine::inject (task::ID id, task::Result result)
  {
    if (!_extracted.erase (id))
    {
      throw std::logic_error ("MapWorkflowEngine::inject: Unknown task.");
    }

    auto const& outputs (result.outputs);

    if (  outputs.size() != 3
       || outputs.count ("input") != 1
       || outputs.count ("output") != 1
       || outputs.count ("N") != 1
       || outputs.at ("input") != id.id
       || outputs.at ("input") + outputs.at ("output") != outputs.at ("N")
       )
    {
      throw std::logic_error ("MapWorkflowEngine::inject: Unexpected result.");
    }
  }
}
