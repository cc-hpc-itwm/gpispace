// Copyright (C) 2011,2015,2020-2023,2025-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <vector>

#include <boost/serialization/string.hpp>
#include <boost/serialization/vector.hpp>

#include <gspc/iml/AllocationHandle.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/vmem/gaspi/pc/type/impl_types.hpp>




      namespace gpi::pc::type::handle
      {
        struct descriptor_t
        {
          gspc::iml::AllocationHandle id{};
          gspc::iml::MemoryOffset offset {0};
          gspc::iml::MemorySize size {0};
          gspc::iml::MemorySize local_size {0};
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
