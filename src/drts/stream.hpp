// Copyright (C) 2023 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#if !GSPC_WITH_IML

#include <gspc/iml/macros.hpp>

static_assert ( gspc::WithIML_v<>
              , GSPC_WITHOUT_IML_ERROR_MESSAGE
              );

#else

#include <gspc/detail/dllexport.hpp>

#include <drts/stream.fwd.hpp>

#include <we/type/value.hpp>

#include <iml/Client.hpp>
#include <iml/MemoryOffset.hpp>
#include <iml/MemoryRegion.hpp>
#include <iml/MemorySize.hpp>
#include <iml/SegmentAndAllocation.hpp>
#include <iml/SharedMemoryAllocation.hpp>

#include <cstddef>
#include <functional>
#include <string>
#include <unordered_set>
#include <utility>

namespace gspc
{
  class GSPC_DLLEXPORT stream
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
    stream ( iml::Client& client
           , iml::SegmentAndAllocation const& buffer
           , iml::MemorySize size_of_slot
           , std::function<void (::pnet::type::value::value_type const&)> on_slot_filled
           );

    iml::Client& _virtual_memory;
    std::function<void (::pnet::type::value::value_type const&)> _on_slot_filled;
    iml::SegmentAndAllocation const& _buffer;
    iml::MemorySize const _size_of_slot;
    std::size_t const _number_of_slots;
    iml::MemoryOffset const _offset_to_meta_data;

    iml::SharedMemoryAllocation _flags;
    iml::SharedMemoryAllocation _update;
    iml::SharedMemoryAllocation _data;

    std::unordered_set<unsigned long> _free_slots;
    std::size_t _sequence_number;
  };
}

#endif
