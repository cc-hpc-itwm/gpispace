#pragma once

#include <iml/client/virtual_memory.hpp>

#include <iml/vmem/gaspi/pc/client/api.hpp>
#include <iml/vmem/gaspi/pc/segment/segment.hpp>
#include <iml/vmem/gaspi/pc/type/flags.hpp>
#include <iml/vmem/gaspi/pc/type/handle.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <boost/format.hpp>

#include <exception>
#include <memory>
#include <string>

namespace iml_client
{
  namespace
  {
    gpi::pc::type::handle_id_t vmem_alloc
      ( std::unique_ptr<gpi::pc::client::api_t> const& api
      , gpi::pc::type::segment_id_t const segment_id
      , unsigned long const size
      , std::string const& description
      )
    {
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
      , _handle_id (vmem_alloc (_api, *_remote_segment, _size, description))
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
