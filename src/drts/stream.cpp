// alexander.petry@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/stream.hpp>

#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/private/drts_impl.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/virtual_memory_impl.hpp>
#include <drts/virtual_memory.hpp>

#include <drts/private/scoped_vmem_cache.hpp>

#include <vmem/intertwine_compat.hpp>
#include <vmem/operations.hpp>

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
                   , gspc::scoped_vmem_segment_and_allocation const& buffer
                   , stream::size_of_slot const& size_of_slot
                   , std::function<void (pnet::type::value::value_type const&)>
                       on_slot_filled
                   )
      : _virtual_memory (drts._->_virtual_memory_api.get())
      , _on_slot_filled (on_slot_filled)
      , _buffer (buffer)
      , _size_of_slot (size_of_slot)
      , _number_of_slots
          (get_number_of_slots_or_throw (buffer.size(), _size_of_slot))
      , _offset_to_meta_data (_number_of_slots * _size_of_slot)
      , _flags (*_virtual_memory, intertwine::vmem::size_t (_number_of_slots))
      , _update (*_virtual_memory, intertwine::vmem::size_t (_number_of_slots))
      , _data (*_virtual_memory, intertwine::vmem::size_t (_size_of_slot))

      , _flags_range
          ( boost::get<intertwine::vmem::mutable_local_range_t>
              ( _virtual_memory->execute_sync
                  ( intertwine::vmem::op::get_mutable_t
                      ( { _buffer._->_data_id
                        , { intertwine::vmem::offset_t (_offset_to_meta_data)
                          , intertwine::vmem::size_t (_number_of_slots)
                          }
                        }
                      , _flags
                      )
                  )
              )
          )

      ,  _data_range (boost::get<intertwine::vmem::mutable_local_range_t>
        ( _virtual_memory->execute_sync
            ( intertwine::vmem::op::allocate_t
                (intertwine::vmem::size_t (_size_of_slot), _data)
            )
        ))

      , _free_slots()
      , _sequence_number (0)
    {
      for (unsigned long slot (0); slot < _number_of_slots; ++slot)
      {
        _free_slots.emplace (slot);
      }
    }
    ~implementation()
    {
      boost::get<intertwine::vmem::void_t>
        ( _virtual_memory->execute_sync
            (intertwine::vmem::op::release_t (_flags_range))
        );

      boost::get<intertwine::vmem::void_t>
        ( _virtual_memory->execute_sync
            (intertwine::vmem::op::release_t (_data_range))
        );
    }

    intertwine::vmem::ipc_client* _virtual_memory;
    std::function<void (pnet::type::value::value_type const&)> _on_slot_filled;
    gspc::scoped_vmem_segment_and_allocation const& _buffer;
    unsigned long const _size_of_slot;
    unsigned long const _number_of_slots;
    unsigned long const _offset_to_meta_data;

    scoped_vmem_cache const _flags;
    scoped_vmem_cache const _update;
    scoped_vmem_cache const _data;

    intertwine::vmem::mutable_local_range_t const _flags_range;
    intertwine::vmem::mutable_local_range_t const _data_range;

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

      char* const flag (static_cast<char*> (_flags_range.pointer()));

      if (_free_slots.empty())
      {
        auto update_range
          ( boost::get<intertwine::vmem::const_local_range_t>
              ( _virtual_memory->execute_sync
                  ( intertwine::vmem::op::get_const_t
                  ({_buffer._->_data_id, {intertwine::vmem::offset_t (_offset_to_meta_data), intertwine::vmem::size_t (_number_of_slots)}}, _update)
                  )
              )
          );


        auto const update (static_cast<char const*> (update_range.pointer()));

        for (unsigned long slot (0); slot < _number_of_slots; ++slot)
        {
          if (update[slot] != flag[slot])
          {
            flag[slot] = update[slot];
            _free_slots.emplace (slot);
          }
        }

      boost::get<intertwine::vmem::void_t>
        ( _virtual_memory->execute_sync
            (intertwine::vmem::op::release_t (update_range))
        );
      }

      if (_free_slots.empty())
      {
        throw std::runtime_error ("No more free slots!");
      }

      unsigned long const slot (*_free_slots.begin());
      _free_slots.erase (_free_slots.begin());

      std::copy ( data.begin(), data.end()
                , static_cast<char*> (_data_range.pointer())
                );

      boost::get<intertwine::vmem::void_t>
        ( _virtual_memory->execute_sync
            ( intertwine::vmem::op::put_t
                ( _data_range
                , { _buffer._->_data_id
                  , { intertwine::vmem::offset_t (slot * _size_of_slot)
                    , intertwine::vmem::size_t (_size_of_slot)
                    }
                  }
                )
            )
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
                 , gspc::scoped_vmem_segment_and_allocation const& buffer
                 , stream::size_of_slot const& size_of_slot
                 , std::function<void (pnet::type::value::value_type const&)>
                     on_slot_filled
                 )
    : _ (new implementation ( drts
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
