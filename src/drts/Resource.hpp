#pragma once

#include <drts/resource/Class.hpp>

#include <boost/optional.hpp>

namespace gspc
{
  class Resource
  {
  public:
    Resource() = default;
    Resource (resource::Class);
    Resource (resource::Class, std::size_t);
    Resource
      ( resource::Class
      , std::size_t
      , boost::optional<std::size_t>
      );
    Resource
      ( resource::Class
      , std::size_t
      , boost::optional<std::size_t>
      , boost::optional<unsigned short>
      );

    resource::Class resource_class;
    std::size_t shm_size;
    boost::optional<std::size_t> socket;
    boost::optional<unsigned short> port;

    template<typename Archive>
      void serialize (Archive& ar, unsigned int)
    {
      ar & resource_class;
      ar & shm_size;
      ar & socket;
      ar & port;
    }

    friend bool operator== (Resource const&, Resource const&);
  };
}
