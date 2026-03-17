// Copyright (C) 2020,2022-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace gspc::iml
{
  template<typename BoostArchive>
    void AllocationHandle::serialize (BoostArchive& archive, unsigned int)
  {
    archive & handle;
  }
}
