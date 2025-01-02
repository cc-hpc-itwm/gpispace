// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <iml/AllocationHandle.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemorySize.hpp>
#include <iml/vmem/gaspi/pc/type/impl_types.hpp>

namespace gpi
{
  namespace pc
  {
    namespace type
    {
      namespace handle
      {
        struct descriptor_t
        {
          iml::AllocationHandle id{};
          iml::MemoryOffset offset {0};
          iml::MemorySize size {0};
          iml::MemorySize local_size {0};
          gpi::pc::type::flags_t flags {is_global::no};

        private:
          friend class ::boost::serialization::access;
          template<typename Archive>
          void serialize (Archive & ar, unsigned int /*version*/)
          {
            ar & id;
            ar & offset;
            ar & size;
            ar & local_size;
            ar & flags;
          }
        };
      }
    }
  }
}
