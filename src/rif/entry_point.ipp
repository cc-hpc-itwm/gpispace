// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace fhg
{
  namespace rif
  {
    template<typename Archive>
      void entry_point::serialize (Archive& ar , unsigned int)
    {
      ar & hostname;
      ar & port;
      ar & pid;
    }
  }
}
