#pragma once

#include <drts/virtual_memory.hpp>

#include <vmem/ipc_client.hpp>
#include <vmem/prefix_filled_data.hpp>
#include <vmem/types.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <vmem/gaspi/equally_distributed_segment_description.hpp>

#include <stdexcept>

namespace fhg
{
  namespace vmem
  {
    struct remote_segment
    {
      remote_segment ( intertwine::vmem::ipc_client& client
                     , intertwine::vmem::size_t size
                     , intertwine::vmem::serialized_segment_description const& info
                     )
        : _client (client)
        , _segment_id (_client.segment_create (size, info))
      {}

      ~remote_segment()
      {
        _client.segment_delete (_segment_id);
      }

      remote_segment (remote_segment const&) = delete;
      remote_segment (remote_segment&&) = delete;
      remote_segment& operator= (remote_segment const&) = delete;
      remote_segment& operator= (remote_segment&&) = delete;

      operator intertwine::vmem::segment_id_t() const
      {
        return _segment_id;
      }

    private:
      intertwine::vmem::ipc_client& _client;
      intertwine::vmem::segment_id_t _segment_id;
    };

    inline std::unique_ptr<remote_segment>
      make_remote_segment ( intertwine::vmem::ipc_client& client
                          , intertwine::vmem::size_t size
                          , gspc::vmem::segment_description description
                          )
    {
      struct visitor_t
        : public boost::static_visitor<std::unique_ptr<remote_segment>>
      {
        std::unique_ptr<remote_segment> operator()
          (gspc::vmem::gaspi_segment_description const& desc) const
        {
          return fhg::util::cxx14::make_unique<remote_segment>
            ( _client
            , _size
            , intertwine::vmem::gaspi::equally_distributed_segment_description
                ( intertwine::vmem::gaspi::communication_buffers
                    ( intertwine::vmem::size_t (desc._communication_buffer_size)
                    , desc._communication_buffer_count
                    )
                )
            );
        }
        std::unique_ptr<remote_segment> operator()
          (gspc::vmem::beegfs_segment_description const&) const
        {
          throw std::logic_error ("NYI: beegfs segment (desc._path)");
        }

        visitor_t ( intertwine::vmem::ipc_client& client
                  , intertwine::vmem::size_t size
                  )
          : _client (client), _size (size) {}
        intertwine::vmem::ipc_client& _client;
        intertwine::vmem::size_t _size;
      } visitor = {client, size};
      return boost::apply_visitor (visitor, description);
    }
  }
}

namespace gspc
{
  struct scoped_vmem_segment_and_allocation::implementation
  {
    implementation ( intertwine::vmem::ipc_client* client
                   , vmem::segment_description segment_desc
                   , unsigned long size
                   )
      : _client (client)
      , _size (size)
      , _remote_segment
          ( fhg::vmem::make_remote_segment ( *_client
                                           , intertwine::vmem::size_t (_size)
                                           , segment_desc
                                           )
          )
      , _data_id ( _client->allocate ( intertwine::vmem::size_t (_size)
                                     , *_remote_segment
                                     //! \todo let pass in!
                                     , intertwine::vmem::prefix_filled_data{}
                                     )
                 )
      , _disowned (false)
    {}
    ~implementation()
    {
      if (!_disowned)
      {
        _client->free (_data_id);
      }
    }
    implementation (implementation const&) = delete;
    implementation (implementation&& other)
      : _client (std::move (other._client))
      , _size (std::move (other._size))
      , _remote_segment (std::move (other._remote_segment))
      , _data_id (std::move (other._data_id))
      , _disowned (std::move (other._disowned))
    {
      other._disowned = true;
    }
    implementation& operator= (implementation const&) = delete;
    implementation& operator= (implementation&&) = delete;

    intertwine::vmem::ipc_client* _client;
    intertwine::vmem::size_t const _size;
    std::unique_ptr<fhg::vmem::remote_segment> _remote_segment;
    intertwine::vmem::data_id_t _data_id;
    bool _disowned;
  };
}
