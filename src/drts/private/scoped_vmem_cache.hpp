// mirko.rahn@itwm.fraunhofer.de

#pragma once

#include <vmem/ipc_client.hpp>

namespace gspc
{
  class scoped_vmem_cache
  {
  public:
    scoped_vmem_cache ( intertwine::vmem::ipc_client& client
                      , intertwine::vmem::size_t size
                      )
      : _client (client)
      , _size (size)
      , _id (_client.shareable_cache_create (size))
    {}
    ~scoped_vmem_cache()
    {
      _client.cache_delete (_id);
    }

    operator intertwine::vmem::cache_id_t const&() const
    {
      return _id;
    }

    intertwine::vmem::size_t const& size() const
    {
      return _size;
    }

  private:
    intertwine::vmem::ipc_client& _client;
    intertwine::vmem::size_t const _size;
    intertwine::vmem::cache_id_t const _id;
  };
}
