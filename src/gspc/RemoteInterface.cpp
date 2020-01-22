#include <gspc/RemoteInterface.hpp>

#include <algorithm>
#include <iterator>
#include <list>
#include <stdexcept>

namespace gspc
{
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
    auto proxy (_proxies.find (resource_id));

    if (proxy != _proxies.end())
    {
      return worker_endpoint_for_scheduler (proxy->second);
    }

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
                  auto fail
                    ( [&] (auto reason)
                      {
                        throw std::runtime_error
                          (str ( boost::format ("Skip start of '%1%': %2%.")
                               % resource.second
                               % reason
                               )
                          );
                      }
                    );

                  if (std::any_of ( children.cbegin()
                                  , children.cend()
                                  , [] (auto const& child)
                                    {
                                      return !std::get<1> (child->second);
                                    }
                                  )
                     )
                  {
                    fail ("Child failure.");
                  }

                  if (resource.second.proxy)
                  {
                    auto is_proxy
                      ( [&] (auto const& child)
                        {
                          return *resource.second.proxy
                            == std::get<0> (child->second).resource_class
                            ;
                        }
                      );

                    auto proxying_child
                      ( std::find_if ( children.cbegin(), children.cend()
                                     , is_proxy
                                     )
                      );

                    if (proxying_child == children.cend())
                    {
                      fail ("Missing proxying child.");
                    }

                    if (std::find_if ( std::next (proxying_child)
                                     , children.cend()
                                     , is_proxy
                                     ) != children.cend()
                       )
                    {
                      fail ("Multiple proxying children.");
                    }

                    return _proxies.emplace
                      ( ++_next_resource_id
                      , std::get<1> ((*proxying_child)->second).value()
                      ).first->first;
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
                if (!_proxies.erase (id.first) && !_workers.erase (id.first))
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
}
