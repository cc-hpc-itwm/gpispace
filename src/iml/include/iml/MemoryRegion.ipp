// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <boost/serialization/base_object.hpp>

namespace iml
{
  template<typename BoostArchive>
    void MemoryRegion::serialize (BoostArchive& archive, unsigned int)
  {
    archive & ::boost::serialization::base_object<MemoryLocation> (*this);
    archive & size;
  }
}
