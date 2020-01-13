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

  bool operator== (Resource const& lhs, Resource const& rhs)
  {
    return std::tie (lhs.resource_class) == std::tie (rhs.resource_class);
  }
  std::ostream& operator<< (std::ostream& os, Resource const& r)
  {
    return os << "resource " << r.resource_class;
  }

  Worker::Worker (Resource resource)
    : _resource {std::move (resource)}
    , _service_dispatcher()
    , _io_service (1)
    // , _add ( _service_dispatcher
    //        , fhg::util::bind_this (this, &Worker::add)
    //        )
    , _service_socket_provider (_io_service, _service_dispatcher)
    , _service_tcp_provider (_io_service, _service_dispatcher)
    , _local_endpoint ( fhg::util::connectable_to_address_string
                          (_service_tcp_provider.local_endpoint())
                      , _service_socket_provider.local_endpoint()
                      )
  {}

  rpc::endpoint const& Worker::local_endpoint() const
  {
    return _local_endpoint;
  }

  RemoteInterface::RemoteInterface (remote_interface::ID id)
    : _next_resource_id {id}
    , _service_dispatcher()
    , _io_service (1)
    , _add ( _service_dispatcher
           , fhg::util::bind_this (this, &RemoteInterface::add)
           )
    , _remove ( _service_dispatcher
              , fhg::util::bind_this (this, &RemoteInterface::remove)
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

  RemoteInterface::WorkerServer::WorkerServer (Resource const& resource)
    : _worker (resource)
  {}
  rpc::endpoint RemoteInterface::WorkerServer::local_endpoint() const
  {
    return _worker.local_endpoint();
  }

  Forest<Resource, ErrorOr<resource::ID>>
    RemoteInterface::add (Forest<Resource> const& resources)
  {
    using ResultNode = forest::Node<Resource, ErrorOr<resource::ID>>;

    return resources.upward_combine_transform
      ( [&] ( forest::Node<Resource> const& resource
            , std::list<ResultNode const*> const& children
            ) -> ResultNode
        {
          if (std::any_of ( children.cbegin()
                          , children.cend()
                          , [] (auto const& child)
                            {
                              return !child->second;
                            }
                          )
             )
          {
            throw std::runtime_error
              (str ( boost::format ("Skip start of '%1%': Child failure.")
                   % resource.first
                   )
              );
          }

          return
            { resource.first
            , _workers.emplace
              ( std::piecewise_construct
              , std::forward_as_tuple (++_next_resource_id)
              , std::forward_as_tuple (resource.first)
              ).first->first
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
          if (!_workers.erase (id.first))
          {
            throw std::invalid_argument
              (str ( boost::format
                       ("RemoteInterface::remove: Unknown worker '%1%'.")
                   % id.first
                   )
              );
          }

          return {id.first, {}};
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
      , remove {*_endpoint}
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

    Forest<Resource, ErrorOr<resource::ID>>
      ConnectionAndPID::add (Forest<Resource> const& resources)
    {
      return _client.add (resources).get();
    }

    Forest<resource::ID, ErrorOr<>>
      ConnectionAndPID::remove (Forest<resource::ID> const& resources)
    {
      return _client.remove (resources).get();
    }
  }
}

namespace gspc
{
  ScopedRuntimeSystem::ScopedRuntimeSystem
      (interface::ResourceManager& resource_manager)
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
                ( std::piecewise_construct
                , std::forward_as_tuple (hostname)
                , std::forward_as_tuple ( _remote_interface_io_service
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

            return &remote_interface->second;
          }
        );
    }

    return remote_interfaces;
  }

  std::unordered_map
    < remote_interface::Hostname
    , ErrorOr<Forest<Resource, ErrorOr<resource::ID>>>
    >
    ScopedRuntimeSystem::add
      ( std::unordered_set<remote_interface::Hostname> hostnames
      , remote_interface::Strategy strategy
      , Forest<Resource> const& resources
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
             , Forest<Resource, ErrorOr<resource::ID>> result
             )
         {
           using Node = forest::Node<Resource, ErrorOr<resource::ID>>;

           //! split (result).first.unordered_transform () -> Resources::Node
           _resource_manager.add
             ( Forest<Resource, ErrorOr<resource::ID>> (result)
             . remove_root_if
                 ([] (Node const& node) { return !node.second; })
             . unordered_transform
                 ( [] (Node const& node)
                   {
                     return interface::ResourceManager::Resources::Node
                       (node.second.value(), node.first.resource_class);
                   }
                 )
             );

           return result;
         };
      //! \todo
      //      |= [&] ( remote_interface::Hostname const& // hostname
      //             , MaybeError<Forest<Resource, ErrorOr<resource::ID>>> // result
      //             )
      //         {
      //           // teardown rif if empty
      //         };
  }

  namespace
  {
    std::unordered_set<resource::ID> child_ts
      (std::list<forest::Node<resource::ID> const*> const& children)
    {
      std::unordered_set<resource::ID> result;
      std::transform ( children.begin(), children.end()
                     , std::inserter (result, result.end())
                     , [] (auto const& node) { return node->first; }
                     );
      return result;
    }

    template<typename Others>
      void require_all_rif_ids_equal
        (remote_interface::ID lhs, Others const& rhs)
    {
      if ( !std::all_of ( rhs.begin()
                        , rhs.end()
                        , [&] (forest::Node<resource::ID> const* child)
                          {
                            return child->first.remote_interface == lhs;
                          }
                        )
         )
      {
        throw std::invalid_argument
          ( "ScopedRuntimeSystem::remove: connected component crosses "
            "remote_interface boundary"
          );
      }
    }
  }

  Forest<resource::ID, ErrorOr<>>
    ScopedRuntimeSystem::remove (Forest<resource::ID> const& to_remove)
  {
    std::unordered_map < remote_interface::Hostname
                       , Forest<resource::ID>
                       > to_remove_by_host;

    //! upward_combine, no transform -- multiway-split by pred
    to_remove.upward_combine_transform
      ( [&] ( forest::Node<resource::ID> const& resource_id
            , std::list<forest::Node<resource::ID> const*> const& children
            )
        {
          auto const rif_id (resource_id.first.remote_interface);
          require_all_rif_ids_equal (rif_id, children);

          auto const hostname (_hostname_by_remote_interface_id.at (rif_id));

          to_remove_by_host[hostname].insert (resource_id, child_ts (children));
          return resource_id;
        }
      );

    Forest<resource::ID, ErrorOr<>> results;

    std::for_each
      ( to_remove_by_host.begin(), to_remove_by_host.end()
      , [&] (auto const& host_and_to_remove)
        {
          //! SAFE: each forest was part of `to_remove`
          results.UNSAFE_merge
            ( _remote_interface_by_hostname
              . at (host_and_to_remove.first)
                . remove (host_and_to_remove.second)
            );
        }
      );

    _resource_manager.remove (to_remove.annotate (resource_classes?!));

    return results;
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
      , Forest<Resource> const& resources
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
              ( [&] (forest::Node<Resource, ErrorOr<resource::ID>> const& node)
                {
                  if (!node.second)
                  {
                    failures.emplace_back
                      ( nest_with_runtime_error
                          ( node.second.error()
                          , "failure on " + host_result.first
                          + " for resource "
                          + boost::lexical_cast<std::string> (node.first)
                          )
                      );
                  }

                  return forest::Node<ErrorOr<resource::ID>> (node.second, {});
                }
              )
          . remove_root_if
              ( [] (forest::Node<ErrorOr<resource::ID>> const& node)
                {
                  return !node.first;
                }
              )
          . unordered_transform
              ( [] (forest::Node<ErrorOr<resource::ID>> const& node)
                {
                  return forest::Node<resource::ID> (node.first.value(), {});
                }
              )
          );
      }
    }

    if (!failures.empty())
    {
      try
      {
        //! \todo No longer throws but returns errorors. Rethrow?!
        remove (successes);
      }
      catch (...)
      {
        failures.emplace_back
          ( nest_with_runtime_error
              ( std::current_exception()
              , "when trying to clean up successfully started resources"
              )
          );
      }

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

    void WithPreferences::remove (Resources to_remove)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      to_remove.for_each_node
        ( [&] (Resources::Node const& resource)
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

      //! \todo Forest::upward_apply
      to_remove.upward_combine_transform
        ( [&] ( Resources::Node const& resource
              , std::list<Resources::Node const*> const& // unused children
              )
          {
            Resources::Node const a = _resources.remove_leaf (resource.first);
            _resource_usage_by_id.erase (resource.first);
            _available_resources_by_class.at (a.second)
              . erase (resource.first)
              ;

            return resource;
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

      //! \note traversal includes `start`
      //! \todo move to Forest or ResourceManager, is likely required
      //! by all resource managers.
      template<typename Forest, typename Callback, typename T>
        void for_each_down_up_reachable_node
          (Forest const& forest, T&& start, Callback&& callback)
      {
        forest.down
          ( start
          , [&] (typename Forest::Node const& forward_dependent)
            {
              forest.up (forward_dependent.first, callback);
            }
          );
      }
    }

    WithPreferences::Acquired WithPreferences::acquire
      (std::list<resource::Class> resource_classes)
    {
      std::unique_lock<std::mutex> resources_lock (_resources_guard);

      auto const has_available_resource
        ( [&] (resource::Class resource_class)
          {
            return !_available_resources_by_class.at (resource_class).empty();
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

      //! \todo ascii art why we block what

      for_each_down_up_reachable_node
        ( _resources
        , requested
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

      for_each_down_up_reachable_node
        ( _resources
        , to_release
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

    void Coallocation::remove (Resources to_remove)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      to_remove.for_each_node
        ( [&] (Resources::Node const& resource)
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

      //! \todo Forest::upward_apply, required: upwards or downwards
      //! per unconnected sub-tree, just *not random*. Every block
      //! that happened before removal has to still block during (or
      //! have one side of the block vanished).
      to_remove.upward_combine_transform
        ( [&] ( Resources::Node const& resource
              , std::list<Resources::Node const*> const& // unused children
              )
          {
            _resource_usage_by_id.erase (resource.first);
            _available_resources_by_class.at (resource.second)
              . erase (resource.first)
              ;
            _resources.remove_leaf (resource.first);
            return resource;
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
            return _available_resources_by_class.at (resource_class).size()
              >= count;
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
            for_each_down_up_reachable_node
              ( _resources
              , requested
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
      for_each_down_up_reachable_node
        ( _resources
        , to_release
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

int main()
try
{
  gspc::resource_manager::Trivial resource_manager;

  gspc::ScopedRuntimeSystem runtime_system (resource_manager);

  gspc::remote_interface::strategy::Thread::State strategy_state;
  gspc::remote_interface::strategy::Thread thread_strategy {&strategy_state};

  auto const resource_ids1
    ( runtime_system.add_or_throw
      ( {"hostname1", "hostname2"}
      , thread_strategy
      , gspc::Forest<gspc::Resource> {}
      )
    );
  FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids1); });

  auto const resource_ids2
    ( runtime_system.add_or_throw
      ( {"hostname3", "hostname4"}
      , thread_strategy
      , gspc::Forest<gspc::Resource> {}
      )
    );
  FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids2); });

  // gspc::remote_interface::strategy::SSH ssh_strategy;

  // //! \note will _not_ use SSH for hostname4 because it already uses THREAD
  // auto const resource_ids3
  //   ( runtime_system.add_or_throw
  //     ( {"hostname4", "hostname6"}
  //     , SSH_strategy
  //     , gspc::Forest<gspc::Resource> {}
  //     )
  //   );
  // FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids3); });

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
