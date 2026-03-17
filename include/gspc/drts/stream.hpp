// Copyright (C) 2014-2015,2020-2023,2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if !GSPC_WITH_IML

#include <gspc/iml/macros.hpp>

static_assert ( gspc::WithIML_v<>
              , GSPC_WITHOUT_IML_ERROR_MESSAGE
              );

#else

#include <gspc/detail/export.hpp>

#include <gspc/drts/stream.fwd.hpp>

#include <gspc/we/type/value.hpp>

#include <gspc/iml/Client.hpp>
#include <gspc/iml/MemoryOffset.hpp>
#include <gspc/iml/MemoryRegion.hpp>
#include <gspc/iml/MemorySize.hpp>
#include <gspc/iml/SegmentAndAllocation.hpp>
#include <gspc/iml/SharedMemoryAllocation.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_set>
#include <utility>

namespace gspc
{
  class GSPC_EXPORT stream
  {
  public:
    void write (std::string const&);

    static void mark_free ( char old_flag_value
                          , std::pair<void*, unsigned long> ptr_to_flag
                          );

    stream() = delete;
    stream (stream const&) = delete;
    stream (stream&&) = default;
    stream& operator= (stream const&) = delete;
    stream& operator= (stream&&) = delete;
    ~stream() = default;

  private:
    friend class scoped_runtime_system;
    stream ( gspc::iml::Client& client
           , gspc::iml::SegmentAndAllocation const& buffer
           , gspc::iml::MemorySize size_of_slot
           , std::function<void (gspc::pnet::type::value::value_type const&)> on_slot_filled
           );

    gspc::iml::Client& _virtual_memory;
    std::function<void (gspc::pnet::type::value::value_type const&)> _on_slot_filled;
    gspc::iml::SegmentAndAllocation const& _buffer;
    gspc::iml::MemorySize const _size_of_slot;
    std::size_t const _number_of_slots;
    gspc::iml::MemoryOffset const _offset_to_meta_data;

    gspc::iml::SharedMemoryAllocation _flags;
    gspc::iml::SharedMemoryAllocation _update;
    gspc::iml::SharedMemoryAllocation _data;

    std::unordered_set<unsigned long> _free_slots;
    std::size_t _sequence_number;
  };
}

#endif
