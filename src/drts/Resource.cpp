#include <drts/Resource.hpp>

namespace gspc
{
  Resource::Resource (resource::Class resource_class_)
    : Resource (resource_class_, 0)
  {}

  Resource::Resource (resource::Class resource_class_, std::size_t shm_size_)
    : Resource (resource_class_, shm_size_, boost::none)
  {}

  Resource::Resource
      ( resource::Class resource_class_
      , std::size_t shm_size_
      , boost::optional<std::size_t> socket_
      )
    : Resource (resource_class_, shm_size_, socket_, boost::none)
  {}

  Resource::Resource
      ( resource::Class resource_class_
      , std::size_t shm_size_
      , boost::optional<std::size_t> socket_
      , boost::optional<unsigned short> port_
      )
    : resource_class (resource_class_)
    , shm_size (shm_size_)
    , socket (socket_)
    , port (port_)
  {}

  bool operator== (Resource const& lhs, Resource const& rhs)
  {
    return std::tie (lhs.resource_class, lhs.shm_size, lhs.socket, lhs.port)
      == std::tie (rhs.resource_class, rhs.shm_size, rhs.socket, rhs.port);
  }
}
