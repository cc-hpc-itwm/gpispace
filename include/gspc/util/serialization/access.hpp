#pragma once

//! Grant friend-access to util-generic's serialization functions,
//! namely by_member.
#define FHG_UTIL_SERIALIZATION_ACCESS(type_)                            \
  FHG_UTIL_SERIALIZATION_ACCESS_IMPL (type_)

#include <gspc/util/serialization/access.ipp>
