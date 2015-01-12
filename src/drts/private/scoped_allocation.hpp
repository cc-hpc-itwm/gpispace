// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_PRIVATE_SCOPED_ALLOCATION_HPP
#define DRTS_PRIVATE_SCOPED_ALLOCATION_HPP

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/handle.hpp>

namespace gspc
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
                       ( description
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
    scoped_allocation
      ( std::unique_ptr<gpi::pc::client::api_t> const& virtual_memory
      , std::string const& description
      , unsigned long size
      )
        : _virtual_memory (virtual_memory)
        , _scoped_segment (scoped_segment (_virtual_memory, description, size))
        , _handle (_virtual_memory->alloc
                    ( _scoped_segment
                    , size
                    , description
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
}

#endif
