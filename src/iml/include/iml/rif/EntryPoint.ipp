// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace iml
{
  namespace rif
  {
    template<typename BoostArchive>
      void EntryPoint::serialize (BoostArchive& archive, unsigned int)
    {
      archive & hostname;
      archive & port;
      archive & pid;
    }
  }
}
