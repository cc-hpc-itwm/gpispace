#include <gspc/remote_interface/ConnectionAndPID.hpp>

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
