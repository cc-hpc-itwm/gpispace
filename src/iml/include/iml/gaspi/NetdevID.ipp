// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace iml
{
  namespace gaspi
  {
    template<typename BoostArchive>
      void NetdevID::serialize (BoostArchive& archive, unsigned int)
    {
      archive & value;
    }
  }
}
