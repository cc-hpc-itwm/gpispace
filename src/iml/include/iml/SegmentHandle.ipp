// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace iml
{
  template<typename BoostArchive>
    void SegmentHandle::serialize (BoostArchive& archive, unsigned int)
  {
    archive & handle;
  }
}
