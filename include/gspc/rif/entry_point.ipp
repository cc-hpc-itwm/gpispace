// Copyright (C) 2021,2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later


  namespace gspc::rif
  {
    template<typename Archive>
      void entry_point::serialize (Archive& ar , unsigned int)
    {
      ar & hostname;
      ar & port;
      ar & pid;
    }
  }
