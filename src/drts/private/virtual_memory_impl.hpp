#pragma once

#include <drts/virtual_memory.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <util-generic/cxx14/make_unique.hpp>

#include <boost/format.hpp>

#include <exception>
#include <memory>
#include <string>

namespace gspc
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

    std::unique_ptr<gpi::pc::client::remote_segment>
      make_remote_segment ( gpi::pc::client::api_t& api
                          , std::size_t size
                          , vmem::segment_description description
                          )
    {
      struct visitor_t
        : public boost::static_visitor<std::unique_ptr<gpi::pc::client::remote_segment>>
      {
        std::unique_ptr<gpi::pc::client::remote_segment> operator()
          (vmem::gaspi_segment_description const& desc) const
        {
          return fhg::util::cxx14::make_unique<gpi::pc::client::remote_segment>
            ( _api
            , gpi::pc::client::remote_segment::gaspi
            , _size
            , desc._communication_buffer_size
            , desc._communication_buffer_count
            );
        }
        std::unique_ptr<gpi::pc::client::remote_segment> operator()
          (vmem::beegfs_segment_description const& desc) const
        {
          return fhg::util::cxx14::make_unique<gpi::pc::client::remote_segment>
            ( _api
            , gpi::pc::client::remote_segment::filesystem
            , _size
            , desc._path
            );
        }

        visitor_t (gpi::pc::client::api_t& api, std::size_t size)
          : _api (api), _size (size) {}
        gpi::pc::client::api_t& _api;
        std::size_t _size;
      } visitor = {api, size};
      return boost::apply_visitor (visitor, description);
    }
  }

  struct scoped_vmem_segment_and_allocation::implementation
  {
    implementation ( std::unique_ptr<gpi::pc::client::api_t> const& api
                   , vmem::segment_description segment_desc
                   , unsigned long size
                   , std::string const& description
                   )
      : _api (api)
      , _size (size)
      , _remote_segment (make_remote_segment (*_api, _size, segment_desc))
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
