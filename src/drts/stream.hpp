// This file is part of GPI-Space.
// Copyright (C) 2021 Fraunhofer ITWM
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#pragma once

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
  class stream
  {
  public:
    void write (std::string const&);

    static void mark_free ( const char old_flag_value
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
