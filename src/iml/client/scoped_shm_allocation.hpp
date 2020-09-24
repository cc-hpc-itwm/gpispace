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
      ( gpi::pc::client::api_t* virtual_memory
      , std::string const& description
      , unsigned long size
      )
        : _virtual_memory (virtual_memory)
        , _shm_allocation ( _virtual_memory->create_shm_segment_and_allocate
                              (description, size)
                          )
        , _size (size)
    {}

    scoped_shm_allocation() = delete;
    scoped_shm_allocation (scoped_shm_allocation const&) = delete;
    scoped_shm_allocation (scoped_shm_allocation&& other)
      : _virtual_memory (other._virtual_memory)
      , _shm_allocation (other._shm_allocation)
      , _size (other._size)
    {
      other._virtual_memory = nullptr;
    }
    scoped_shm_allocation& operator= (scoped_shm_allocation const&) = delete;
    scoped_shm_allocation& operator= (scoped_shm_allocation&&) = delete;
    ~scoped_shm_allocation()
    {
      if (_virtual_memory)
      {
        _virtual_memory->free_and_delete_shm_segment (_shm_allocation);
      }
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
    gpi::pc::client::api_t* _virtual_memory;
    gpi::pc::client::api_t::shm_allocation const _shm_allocation;
    unsigned long const _size;
  };
  }
}
