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

#include <drts/stream.hpp>

#include <drts/drts_iml.hpp>

#include <boost/format.hpp>

#include <algorithm>
#include <stdexcept>

namespace gspc
{
  namespace
  {
    std::size_t get_number_of_slots_or_throw
      (iml::MemorySize const available, iml::MemorySize const size_of_slot)
    {
      std::size_t const number_of_slots (available / (size_of_slot + 1));
      if (0 == number_of_slots)
      {
        throw std::logic_error
          ( "allocated memory is too small to hold slots and meta data. At least "
          + std::to_string (size_of_slot + 1) + " bytes are required."
          );
      }
      return number_of_slots;
    }
  }

  stream::stream
      ( iml::Client& client
      , iml::SegmentAndAllocation const& buffer
      , iml::MemorySize size_of_slot
      , std::function<void (::pnet::type::value::value_type const&)>
          on_slot_filled
      )
    : _virtual_memory (client)
    , _on_slot_filled (std::move (on_slot_filled))
    , _buffer (buffer)
    , _size_of_slot (size_of_slot)
    , _number_of_slots
        (get_number_of_slots_or_throw (buffer.size(), _size_of_slot))
    , _offset_to_meta_data (_number_of_slots * _size_of_slot)
    , _flags (_virtual_memory, _number_of_slots)
    , _update (_virtual_memory, _number_of_slots)
    , _data (_virtual_memory, _size_of_slot)
    , _free_slots()
    , _sequence_number (0)
  {
    _virtual_memory.memcpy
      ( _flags.memory_location()
      , _buffer.memory_location (_offset_to_meta_data)
      , _number_of_slots
      );

    for (unsigned long slot (0); slot < _number_of_slots; ++slot)
    {
      _free_slots.emplace (slot);
    }
  }

  void stream::write (std::string const& data)
  {
    if (data.size() > _size_of_slot)
    {
      throw std::invalid_argument
        ( ( ::boost::format ("data size > slot size (%1% > %2%")
          % data.size()
          % _size_of_slot
          ).str()
        );
    }

    char* const flag (_flags.pointer());

    if (_free_slots.empty())
    {
      _virtual_memory.memcpy
        ( _update.memory_location()
        , _buffer.memory_location (_offset_to_meta_data)
        , _number_of_slots
        );

      char const* const update (_update.pointer());

      for (unsigned long slot (0); slot < _number_of_slots; ++slot)
      {
        if (update[slot] != flag[slot])
        {
          flag[slot] = update[slot];
          _free_slots.emplace (slot);
        }
      }
    }

    if (_free_slots.empty())
    {
      throw std::runtime_error ("No more free slots!");
    }

    unsigned long const slot (*_free_slots.begin());
    _free_slots.erase (_free_slots.begin());

    char* const content (_data.pointer());

    std::copy (data.begin(), data.end(), content);

    _virtual_memory.memcpy
      ( _buffer.memory_location (slot * _size_of_slot)
      , _data.memory_location()
      , data.size()
      );

    _on_slot_filled
      ( pnet::vmem::stream_slot_to_value
          ( _buffer.memory_region (_offset_to_meta_data + slot, 1UL)
          , _buffer.memory_region (slot * _size_of_slot, data.size())
          , flag[slot]
          , _sequence_number++
          )
      );
  }

  void stream::mark_free ( char old_flag_value
                         , std::pair<void*, unsigned long> ptr_to_flag
                         )
  {
    *static_cast<char*> (ptr_to_flag.first) = 1 - old_flag_value;
  }
}
