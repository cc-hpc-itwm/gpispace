// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

namespace we
{
  namespace type
  {
    template<class Archive>
      void memory_transfer::serialize (Archive& ar, unsigned int)
    {
      ar & _global;
      ar & _local;
      ar & _not_modified_in_module_call;
      ar & _allow_empty_ranges;
    }
  }
}
