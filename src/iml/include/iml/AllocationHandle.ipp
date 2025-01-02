// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace iml
{
  template<typename BoostArchive>
    void AllocationHandle::serialize (BoostArchive& archive, unsigned int)
  {
    archive & handle;
  }
}
