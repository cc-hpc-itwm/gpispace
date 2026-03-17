// Copyright (C) 2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/serialization/base_object.hpp>

namespace gspc::iml
{
  template<typename BoostArchive>
    void SharedMemoryAllocationHandle::serialize
      (BoostArchive& archive, unsigned int)
  {
    archive & ::boost::serialization::base_object<AllocationHandle> (*this);
  }
}
