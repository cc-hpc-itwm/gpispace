#pragma once

#include <drts/virtual_memory.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <exception>
#include <memory>
#include <string>

namespace gspc
{
  namespace
  {
    gpi::pc::type::handle_id_t vmem_alloc
      ( std::unique_ptr<gpi::pc::client::api_t> const& api
      , unsigned long const size
      , std::string const& description
      )
    {
      gpi::pc::type::segment_id_t const segment_id (1);

      gpi::pc::type::handle_id_t handle_id
        (api->alloc ( segment_id
                    , size
                    , description
                    , gpi::pc::F_GLOBAL
                    )
        );

      if (gpi::pc::type::handle::is_null (handle_id))
      {
        throw std::runtime_error
          ( ( boost::format
              ("Could not allocate %1% bytes with description '%2%'")
            % size
            % description
            ).str()
          );
      }

      return handle_id;
    }
  }

  struct vmem_allocation::implementation
  {
    implementation ( std::unique_ptr<gpi::pc::client::api_t> const& api
                   , unsigned long size
                   , std::string const& description
                   )
      : _api (api)
      , _size (size)
      , _handle_id (vmem_alloc (_api, _size, description))
      , _disowned (false)
    {}
    ~implementation()
    {
      if (!_disowned)
      {
        _api->free (gpi::pc::type::handle_t (_handle_id));
      }
    }
    implementation (implementation&& other)
      : _api (std::move (other._api))
      , _size (std::move (other._size))
      , _handle_id (std::move (other._handle_id))
      , _disowned (std::move (other._disowned))
    {
      other._disowned = true;
    }

    std::unique_ptr<gpi::pc::client::api_t> const& _api;
    unsigned long const _size;
    gpi::pc::type::handle_id_t _handle_id;
    bool _disowned;
  };
}
