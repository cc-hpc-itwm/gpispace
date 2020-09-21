#pragma once

#include <iml/client/virtual_memory.hpp>

#include <iml/vmem/gaspi/pc/client/api.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <exception>
#include <memory>
#include <string>

namespace iml_client
{
  struct vmem_allocation::implementation
  {
    implementation ( std::unique_ptr<gpi::pc::client::api_t> const& api
                   , vmem::segment_description segment_desc
                   , unsigned long size
                   , std::string const& description
                   )
      : _api (api)
      , _size (size)
      , _remote_segment
          ( fhg::util::cxx14::make_unique<gpi::pc::client::remote_segment>
              (*_api, segment_desc, _size)
          )
      , _handle_id (_api->alloc (*_remote_segment, _size, description))
      , _disowned (false)
    {}
    ~implementation()
    {
      if (!_disowned)
      {
        _api->free (gpi::pc::type::handle_t (_handle_id));
      }
    }
    implementation (implementation const&) = delete;
    implementation (implementation&& other)
      : _api (std::move (other._api))
      , _size (std::move (other._size))
      , _remote_segment (std::move (other._remote_segment))
      , _handle_id (std::move (other._handle_id))
      , _disowned (std::move (other._disowned))
    {
      other._disowned = true;
    }
    implementation& operator= (implementation const&) = delete;
    implementation& operator= (implementation&&) = delete;

    std::unique_ptr<gpi::pc::client::api_t> const& _api;
    unsigned long const _size;
    std::unique_ptr<gpi::pc::client::remote_segment> _remote_segment;
    gpi::pc::type::handle_id_t _handle_id;
    bool _disowned;
  };
}
