#pragma once

#include <iml/vmem/gaspi/pc/client/api.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>

namespace iml
{
  namespace client
  {
  class scoped_shm_allocation
  {
  public:
    scoped_shm_allocation
      ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
      , std::string const& description
      , unsigned long size
      )
        : _virtual_memory (virtual_memory)
        , _shm_allocation ( _virtual_memory->create_shm_segment_and_allocate
                              (description, size)
                          )
        , _size (size)
    {}

    ~scoped_shm_allocation()
    {
      _virtual_memory->free_and_delete_shm_segment (_shm_allocation);
    }

    operator gpi::pc::type::handle_t() const
    {
      return _shm_allocation.second;
    }

    unsigned long size() const
    {
      return _size;
    }

  private:
    std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
    gpi::pc::client::api_t::shm_allocation const _shm_allocation;
    unsigned long const _size;
  };
  }
}
