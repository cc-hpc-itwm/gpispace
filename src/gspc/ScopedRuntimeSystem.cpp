#include <gspc/ScopedRuntimeSystem.hpp>

#include <util-generic/nest_exceptions.hpp>
#include <util-generic/wait_and_collect_exceptions.hpp>

#include <boost/lexical_cast.hpp>

#include <exception>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

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
