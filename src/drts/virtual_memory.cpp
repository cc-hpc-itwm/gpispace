// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/virtual_memory.hpp>

#include <drts/private/drts_impl.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/scoped_allocation.hpp>

#include <we/type/value/poke.hpp>

#include <gpi-space/pc/client/api.hpp>
#include <gpi-space/pc/segment/segment.hpp>
#include <gpi-space/pc/type/flags.hpp>
#include <gpi-space/pc/type/handle.hpp>

#include <boost/format.hpp>

#include <exception>

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

  vmem_allocation::vmem_allocation ( scoped_runtime_system const* const drts
                                   , unsigned long size
                                   , std::string const& description
                                   )
    : _ ( new vmem_allocation::implementation
          (drts->_->_virtual_memory_api, size, description)
        )
  {}
  vmem_allocation::vmem_allocation ( scoped_runtime_system const* const drts
                                   , unsigned long size
                                   , std::string const& description
                                   , char const* const data
                                   )
    : vmem_allocation (drts, size, description)
  {
    scoped_allocation const buffer
      (drts->_->_virtual_memory_api, "vmem_allocation_buffer", size);

    char* const content
      (static_cast<char* const> (drts->_->_virtual_memory_api->ptr (buffer)));
    std::copy (data, data + size, content);

    drts->_->_virtual_memory_api->wait
      ( drts->_->_virtual_memory_api->memcpy
        ( {_->_handle_id, 0}
        , {buffer, 0}
        , size
        )
      );
  }
  PIMPL_DTOR (vmem_allocation)

  std::string const vmem_allocation::handle() const
  {
    // taken from gpi-space/pc/type/handle.hpp
    std::ostringstream oss;

    oss << "0x";
    oss.flags (std::ios::hex);
    oss.width (18);
    oss.fill ('0');
    oss << _->_handle_id;

    return oss.str();
  }
  std::size_t vmem_allocation::size() const
  {
    return _->_size;
  }
  pnet::type::value::value_type vmem_allocation::global_memory_range
    ( std::size_t const offset
    , std::size_t const size
    ) const
  {
    if ((offset + size) > _->_size)
    {
      throw std::logic_error
        ((boost::format ("slice [%1%, %2%) is outside of allocation")
         % offset % (offset + size)
         ).str()
        );
    }

    pnet::type::value::value_type range;
    pnet::type::value::poke (std::list<std::string> {"handle", "name"}, range, handle());
    pnet::type::value::poke (std::list<std::string> {"offset"}, range, offset);
    pnet::type::value::poke (std::list<std::string> {"size"}, range, size);

    return range;
  }
  pnet::type::value::value_type vmem_allocation::global_memory_range() const
  {
    return global_memory_range (0UL, _->_size);
  }
  vmem_allocation::vmem_allocation (vmem_allocation&& other)
    : _ (std::move (other._))
  {}
}
