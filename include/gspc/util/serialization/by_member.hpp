#pragma once

#include <gspc/util/serialization/detail/base_class.hpp>



    namespace gspc::util::serialization
    {
      //! Serialize a type_ by serializing it's (publically
      //! accessible) base_or_members_... individually.
      //! \note Requires that type_ is default constructible.
      //! \see base_class<T>()
      //! \todo Should we ensure that base classes are deserialized
      //! before the class itself?
#define FHG_UTIL_SERIALIZATION_BY_MEMBER(type_, base_or_members_...)    \
      FHG_UTIL_SERIALIZATION_DETAIL_BY_MEMBER (type_, base_or_members_)
    }



#include <gspc/util/serialization/by_member.ipp>
