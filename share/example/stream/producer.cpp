// mirko.rahn@itwm.fraunhofer.de

#include <share/example/stream/producer.hpp>

#include <we/type/value/peek.hpp>
#include <we/type/value/poke.hpp>

#include <drts/client.hpp>
#include <drts/virtual_memory.hpp>

#include <fhg/util/make_unique.hpp>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>

#include <stdexcept>

namespace stream
{
  producer::scoped_allocation::scoped_allocation
    ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
    , std::string const& description
    , unsigned long size
    )
      : _virtual_memory (virtual_memory)
      , _segment ( _virtual_memory->register_segment
                   ( "stream_producer_" + description
                   , size
                   , gpi::pc::F_EXCLUSIVE | gpi::pc::F_FORCE_UNLINK
                   )
                 )
      , _handle (_virtual_memory->alloc ( _segment
                                        , size
                                        , "stream_producer_" + description
                                        , gpi::pc::F_EXCLUSIVE
                                        )
                )
  {}
  producer::scoped_allocation::~scoped_allocation()
  {
    _virtual_memory->free (_handle);
    _virtual_memory->unregister_segment (_segment);
  }

  producer::producer
    ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
    , gspc::vmem_allocation const& buffer
    , gspc::vmem_allocation const& meta
    , unsigned long num_slot
    , unsigned long size_slot
    , gspc::client* client
    , sdpa::job_id_t const& job_id
    , std::string const& place_name
    )
      : _virtual_memory (virtual_memory)
      , _buffer (buffer)
      , _meta (meta)
      , _num_slot (num_slot)
      , _size_slot (size_slot)
      , _client (client)
      , _job_id (job_id)
      , _place_name (place_name)
      , _flags (_virtual_memory, "flag", _num_slot)
      , _update (_virtual_memory, "update", _num_slot)
      , _data (_virtual_memory, "data", _size_slot)
      , _free_slots()
  {
    _virtual_memory->wait
      ( _virtual_memory->memcpy
        ( {_flags, 0}
        , {boost::lexical_cast<gpi::pc::type::handle_t> (_meta.handle()), 0}
        , num_slot
        , gpi::pc::type::queue_id_t()
        )
      );

    for (unsigned long slot (0); slot < _num_slot; ++slot)
    {
      _free_slots.emplace (slot);
    }
  }

  void producer::produce (std::string const& data)
  {
    if (data.size() > _size_slot)
    {
      throw std::invalid_argument
        (( boost::format ("data size > slot size (%1% > %2%")
         % data.size()
         % _size_slot
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
          , _num_slot
          , gpi::pc::type::queue_id_t()
          )
        );

      char const* const update
        (static_cast<char*> (_virtual_memory->ptr (_update)));

      for (unsigned long slot (0); slot < _num_slot; ++slot)
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
          , slot * _size_slot
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
      pnet::type::value::poke ("offset", range, slot * _size_slot);
      pnet::type::value::poke ("size", range, data.size());
      pnet::type::value::poke ("data", value, range);
    }
    pnet::type::value::poke ("flag", value, flag[slot]);

    _client->put_token (_job_id, _place_name, value);
  }
}
