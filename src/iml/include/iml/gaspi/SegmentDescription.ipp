// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace iml
{
  namespace gaspi
  {
    template<typename BoostArchive>
      void SegmentDescription::serialize (BoostArchive& archive, unsigned int)
    {
      archive & communication_buffer_size;
      archive & communication_buffer_count;
    }
  }
}
