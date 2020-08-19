#pragma once

#include <drts/resource/Class.hpp>

#include <drts/util-generic_hash_forward_declare.hpp>

namespace gspc
{
  class Resource
  {
  public:
    Resource() = default;
    Resource (resource::Class);
    Resource (resource::Class, std::size_t);

    //! \todo individual-worker-specific, non-resource attributes,
    //! e.g. socket binding, port, memory…
    resource::Class resource_class;
    std::size_t shm_size;

    template<typename Archive>
      void serialize (Archive& ar, unsigned int);

    friend std::ostream& operator<< (std::ostream&, Resource const&);
    friend bool operator== (Resource const&, Resource const&);
  };
}

UTIL_MAKE_COMBINED_STD_HASH_DECLARE (gspc::Resource);

#include <drts/Resource.ipp>
