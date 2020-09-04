// This file is part of GPI-Space.
// Copyright (C) 2020 Fraunhofer ITWM
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

#include <drts/drts.hpp>
#include <drts/stream.hpp>

#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/private/drts_impl.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/virtual_memory_impl.hpp>
#include <drts/virtual_memory.hpp>

#include <drts/private/scoped_allocation.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>

#include <boost/format.hpp>

#include <atomic>
#include <unordered_set>

namespace gspc
{
  struct stream::implementation
  {
    std::size_t get_number_of_slots_or_throw ( std::size_t const available
                                             , std::size_t const size_of_slot
                                             )
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

    implementation ( scoped_runtime_system const& drts
                   , std::string const& name
                   , gspc::vmem_allocation const& buffer
                   , stream::size_of_slot const& size_of_slot
                   , std::function<void (pnet::type::value::value_type const&)>
                       on_slot_filled
                   )
      : _virtual_memory (drts._->_virtual_memory_api)
      , _on_slot_filled (on_slot_filled)
      , _buffer (buffer)
      , _size_of_slot (size_of_slot)
      , _number_of_slots
          (get_number_of_slots_or_throw (buffer.size(), _size_of_slot))
      , _offset_to_meta_data (_number_of_slots * _size_of_slot)
      , _flags
        (_virtual_memory, "stream_producer_flags_" + name, _number_of_slots)
      , _update
        (_virtual_memory, "stream_producer_update_" + name, _number_of_slots)
      , _data
        (_virtual_memory, "stream_producer_data_" + name, _size_of_slot)
      , _free_slots()
      , _sequence_number (0)
    {
      _virtual_memory->memcpy_and_wait
        ( {_flags, 0}
        , {_buffer._->_handle_id, _offset_to_meta_data}
        , _number_of_slots
        );

      for (unsigned long slot (0); slot < _number_of_slots; ++slot)
      {
        _free_slots.emplace (slot);
      }
    }

    std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
    std::function<void (pnet::type::value::value_type const&)> _on_slot_filled;
    gspc::vmem_allocation const& _buffer;
    unsigned long const _size_of_slot;
    unsigned long const _number_of_slots;
    unsigned long const _offset_to_meta_data;

    scoped_allocation const _flags;
    scoped_allocation const _update;
    scoped_allocation const _data;

    std::unordered_set<unsigned long> _free_slots;
    std::atomic<std::size_t> _sequence_number;

    void write (std::string const& data)
    {
      if (data.size() > _size_of_slot)
      {
        throw std::invalid_argument
          (( boost::format ("data size > slot size (%1% > %2%")
           % data.size()
           % _size_of_slot
           ).str()
          );
      }

      char* const flag
        (static_cast<char*> (_virtual_memory->ptr (_flags)));

      if (_free_slots.empty())
      {
        _virtual_memory->memcpy_and_wait
          ( {_update, 0}
          , {_buffer._->_handle_id, _offset_to_meta_data}
          , _number_of_slots
          );

        char const* const update
          (static_cast<char*> (_virtual_memory->ptr (_update)));

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

      char* const content
        (static_cast<char*> (_virtual_memory->ptr (_data)));

      std::copy (data.begin(), data.end(), content);

      _virtual_memory->memcpy_and_wait
        ( {_buffer._->_handle_id, slot * _size_of_slot}
        , {_data, 0}
        , data.size()
        );

      pnet::type::value::value_type value;
      pnet::type::value::poke ( "meta"
                              , value
                              , _buffer.global_memory_range
                                ( _offset_to_meta_data + slot
                                , 1UL
                                )
                              );
      pnet::type::value::poke ( "data"
                              , value
                              , _buffer.global_memory_range ( slot * _size_of_slot
                                                            , data.size()
                                                            )
                              );
      pnet::type::value::poke ("flag", value, flag[slot]);
      pnet::type::value::poke ("id", value, _sequence_number++);

      _on_slot_filled (value);
    }
  };

  stream::stream ( scoped_runtime_system const& drts
                 , std::string const& name
                 , gspc::vmem_allocation const& buffer
                 , stream::size_of_slot const& size_of_slot
                 , std::function<void (pnet::type::value::value_type const&)>
                     on_slot_filled
                 )
    : _ (new implementation ( drts
                            , name
                            , buffer
                            , size_of_slot
                            , on_slot_filled
                            )
        )
  {}

  stream::stream (stream&& other)
    : _ (std::move (other._))
  {}

  PIMPL_DTOR (stream)

  void stream::write (std::string const& data)
  {
    _->write (data);
  }

  void stream::mark_free ( const char old_flag_value
                         , std::pair<void*, unsigned long> ptr_to_flag
                         )
  {
    *static_cast<char*> (ptr_to_flag.first) = 1 - old_flag_value;
  }
}
