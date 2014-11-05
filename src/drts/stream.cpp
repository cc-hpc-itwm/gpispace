// alexander.petry@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/stream.hpp>

#include <drts/client.fwd.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/virtual_memory.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <fhg/util/make_unique.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>

#include <boost/format.hpp>

#include <unordered_set>

namespace gspc
{
  struct stream::implementation
  {
    class scoped_allocation
    {
      class scoped_segment
      {
      public:
        scoped_segment
          ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
          , std::string const& description
          , unsigned long size
          )
            : _virtual_memory (virtual_memory)
            , _segment (_virtual_memory->register_segment
                         ( "stream_producer_" + description
                         , size
                         , gpi::pc::F_EXCLUSIVE | gpi::pc::F_FORCE_UNLINK
                         )
                       )
        {}

        ~scoped_segment()
        {
          _virtual_memory->unregister_segment (_segment);
        }

        operator gpi::pc::type::handle_id_t const& () const
        {
          return _segment;
        }

      private:
        std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
        gpi::pc::type::handle_id_t const _segment;
      };

    public:
      scoped_allocation ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
                        , std::string const& description
                        , unsigned long size
                        )
        : _virtual_memory (virtual_memory)
        , _scoped_segment (scoped_segment (_virtual_memory, description, size))
        , _handle (_virtual_memory->alloc
                    ( _scoped_segment
                    , size
                    , "stream_producer_" + description
                    , gpi::pc::F_EXCLUSIVE
                    )
                  )
      {}

      ~scoped_allocation()
      {
        _virtual_memory->free (_handle);
      }

      operator gpi::pc::type::handle_t const& () const
      {
        return _handle;
      }

    private:
      std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
      scoped_segment const _scoped_segment;
      gpi::pc::type::handle_t const _handle;
    };

    implementation ( scoped_runtime_system const& drts
                   , std::string const& name
                   , gspc::vmem_allocation const& buffer
                   , gspc::vmem_allocation const& meta
                   , stream::size_of_slot const& size_of_slot
                   , stream::number_of_slots const& number_of_slots
                   , std::function<void (pnet::type::value::value_type const&)> on_slot_filled
                   )
      : _virtual_memory (drts.virtual_memory_api())
      , _on_slot_filled (on_slot_filled)
      , _buffer (buffer)
      , _meta (meta)
      , _size_of_slot (size_of_slot)
      , _number_of_slots (number_of_slots)
      , _flags (_virtual_memory, "flags_" + name, _number_of_slots * size_of_meta_data_slot())
      , _update (_virtual_memory, "update_" + name, _number_of_slots * size_of_meta_data_slot())
      , _data (_virtual_memory, "data_" + name, _size_of_slot * size_of_meta_data_slot())
      , _free_slots()
    {
      _virtual_memory->wait
        ( _virtual_memory->memcpy
          ( {_flags, 0}
          , {boost::lexical_cast<gpi::pc::type::handle_t> (_meta.handle()), 0}
          , _number_of_slots
          , gpi::pc::type::queue_id_t()
          )
        );

      for (unsigned long slot (0); slot < _number_of_slots; ++slot)
      {
        _free_slots.emplace (slot);
      }
    }

    std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
    std::function<void (pnet::type::value::value_type const&)> _on_slot_filled;
    gspc::vmem_allocation const& _buffer;
    gspc::vmem_allocation const& _meta;
    unsigned long const _size_of_slot;
    unsigned long const _number_of_slots;
    gspc::job_id_t const _job_id;
    std::string const _place_name;

    scoped_allocation const _flags;
    scoped_allocation const _update;
    scoped_allocation const _data;

    std::unordered_set<unsigned long> _free_slots;

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
        _virtual_memory->wait
          ( _virtual_memory->memcpy
            ( {_update, 0}
            , {boost::lexical_cast<gpi::pc::type::handle_t> (_meta.handle()), 0}
            , _number_of_slots
            , gpi::pc::type::queue_id_t()
            )
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

      _virtual_memory->wait
        ( _virtual_memory->memcpy
          ( { boost::lexical_cast<gpi::pc::type::handle_t> (_buffer.handle())
            , slot * _size_of_slot
            }
          , {_data, 0}
          , data.size()
          , gpi::pc::type::queue_id_t()
          )
        );

      pnet::type::value::value_type value;
      {
        pnet::type::value::value_type range (_meta.global_memory_range());
        pnet::type::value::poke ("offset", range, slot);
        pnet::type::value::poke ("size", range, 1UL);
        pnet::type::value::poke ("meta", value, range);
      }
      {
        pnet::type::value::value_type range (_buffer.global_memory_range());
        pnet::type::value::poke ("offset", range, slot * _size_of_slot);
        pnet::type::value::poke ("size", range, data.size());
        pnet::type::value::poke ("data", value, range);
      }
      pnet::type::value::poke ("flag", value, flag[slot]);

      _on_slot_filled (value);
    }
  };

  stream::stream ( scoped_runtime_system const& drts
                 , std::string const& name
                 , gspc::vmem_allocation const& buffer
                 , gspc::vmem_allocation const& meta
                 , stream::size_of_slot const& size_of_slot
                 , stream::number_of_slots const& number_of_slots
                 , std::function<void (pnet::type::value::value_type const&)> on_slot_filled
                 )
    : _ (fhg::util::make_unique<implementation>
          ( drts
          , name
          , buffer
          , meta
          , size_of_slot
          , number_of_slots
          , on_slot_filled
          )
        )
  {}

  stream::stream (stream&& other)
    : _ (std::move (other._))
  {}

  stream::~stream()
  {}

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

  std::size_t stream::size_of_meta_data_slot()
  {
    return sizeof (char);
  }
}
