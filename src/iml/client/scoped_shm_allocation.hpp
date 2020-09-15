#pragma once

#include <iml/vmem/gaspi/pc/client/api.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>

namespace iml
{
  namespace client
  {
  class scoped_shm_allocation
  {
    class scoped_shm_segment
    {
    public:
      scoped_shm_segment
        ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
        , std::string const& description
        , unsigned long size
        )
          : _virtual_memory (virtual_memory)
          , _segment (_virtual_memory->register_segment (description, size))
      {}

      ~scoped_shm_segment()
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
    scoped_shm_allocation
      ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
      , std::string const& description
      , unsigned long size
      )
        : _virtual_memory (virtual_memory)
        , _scoped_shm_segment (scoped_shm_segment (_virtual_memory, description, size))
        , _handle (_virtual_memory->alloc
                    ( _scoped_shm_segment
                    , size
                    , description
                    , gpi::pc::is_global::no
                    , 0 // NOT global
                    )
                  )
        , _size (size)
    {}

    ~scoped_shm_allocation()
    {
      _virtual_memory->free (_handle);
    }

    operator gpi::pc::type::handle_t const& () const
    {
      return _handle;
    }

    unsigned long size() const
    {
      return _size;
    }

  private:
    std::unique_ptr<gpi::pc::client::api_t> const& _virtual_memory;
    scoped_shm_segment const _scoped_shm_segment;
    gpi::pc::type::handle_t const _handle;
    unsigned long const _size;
  };
  }
}
