// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace iml
{
  template<typename BoostArchive>
    void MemoryLocation::serialize (BoostArchive& archive, unsigned int)
  {
    archive & allocation;
    archive & offset;
  }
}
