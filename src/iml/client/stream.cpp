#include <iml/client/iml.hpp>
#include <iml/client/stream.hpp>

#include <iml/client/iml.fwd.hpp>
#include <iml/client/private/iml_impl.hpp>
#include <iml/client/private/pimpl.hpp>
#include <iml/client/private/virtual_memory_impl.hpp>
#include <iml/client/virtual_memory.hpp>

#include <iml/client/scoped_shm_allocation.hpp>

#include <iml/vmem/gaspi/pc/client/api.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>

#include <boost/format.hpp>

#include <atomic>
#include <unordered_set>

namespace iml_client
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

    implementation ( scoped_iml_runtime_system const& drts
                   , std::string const& name
                   , iml_client::vmem_allocation const& buffer
                   , stream::size_of_slot const& size_of_slot
                   , std::function<void ( gpi::pc::type::range_t const metadata
                                        , gpi::pc::type::range_t const data
                                        , char const flag
                                        , std::size_t const id
                                        )
                                  > on_slot_filled
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
    std::function<void ( gpi::pc::type::range_t const metadata
                       , gpi::pc::type::range_t const data
                       , char const flag
                       , std::size_t const id
                       )
                 > _on_slot_filled;
    iml_client::vmem_allocation const& _buffer;
    unsigned long const _size_of_slot;
    unsigned long const _number_of_slots;
    unsigned long const _offset_to_meta_data;

    iml::client::scoped_shm_allocation const _flags;
    iml::client::scoped_shm_allocation const _update;
    iml::client::scoped_shm_allocation const _data;

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

      _on_slot_filled ( { _buffer._->_handle_id
                        , _offset_to_meta_data + slot
                        , 1UL
                        }
                      , { _buffer._->_handle_id
                        , slot * _size_of_slot
                        , data.size()
                        }
                      , flag[slot]
                      , _sequence_number++
                      );
    }
  };

  stream::stream ( scoped_iml_runtime_system const& drts
                 , std::string const& name
                 , iml_client::vmem_allocation const& buffer
                 , stream::size_of_slot const& size_of_slot
                 , std::function<void ( gpi::pc::type::range_t const metadata
                                      , gpi::pc::type::range_t const data
                                      , char const flag
                                      , std::size_t const id
                                      )
                                > on_slot_filled
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
