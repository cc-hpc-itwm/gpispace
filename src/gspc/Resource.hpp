#pragma once

#include <gspc/resource/Class.hpp>

#include <gspc/util-generic_hash_forward_declare.hpp>

namespace gspc
{
  class Resource
  {
  public:
    Resource() = default;
    Resource (resource::Class);

    //! \todo individual-worker-specific, non-resource attributes,
    //! e.g. socket binding, port, memory…
    resource::Class resource_class;

    template<typename Archive>
      void serialize (Archive& ar, unsigned int);

    friend std::ostream& operator<< (std::ostream&, Resource const&);
    friend bool operator== (Resource const&, Resource const&);
  };
}

UTIL_MAKE_COMBINED_STD_HASH_DECLARE (gspc::Resource);

#include <gspc/Resource.ipp>
