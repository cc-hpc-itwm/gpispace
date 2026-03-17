// Copyright (C) 2020,2022-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later


  namespace gspc::iml::gaspi
  {
    template<typename BoostArchive>
      void SegmentDescription::serialize (BoostArchive& archive, unsigned int)
    {
      archive & communication_buffer_size;
      archive & communication_buffer_count;
    }
  }
