#include <iml/client/iml.hpp>
#include <iml/client/virtual_memory.hpp>

#include <iml/client/private/iml_impl.hpp>
#include <iml/client/private/pimpl.hpp>
#include <iml/client/scoped_shm_allocation.hpp>
#include <iml/client/private/virtual_memory_impl.hpp>

#include <boost/format.hpp>

#include <exception>
#include <sstream>

namespace iml_client
{
  vmem_allocation::vmem_allocation ( scoped_iml_runtime_system const* const drts
                                   , vmem::segment_description segment_desc
                                   , unsigned long size
                                   , std::string const& description
                                   )
    : _ ( new vmem_allocation::implementation
            (drts->_->_virtual_memory_api, segment_desc, size, description)
        )
  {}
  vmem_allocation::vmem_allocation ( scoped_iml_runtime_system const* const drts
                                   , vmem::segment_description segment_desc
                                   , unsigned long size
                                   , std::string const& description
                                   , char const* const data
                                   )
    : vmem_allocation (drts, segment_desc, size, description)
  {
    iml::client::scoped_shm_allocation const buffer
      (drts->_->_virtual_memory_api, "vmem_allocation_buffer", size);

    char* const content
      (static_cast<char*> (drts->_->_virtual_memory_api->ptr (buffer)));
    std::copy (data, data + size, content);

    drts->_->_virtual_memory_api->memcpy_and_wait
      ( {_->_handle_id, 0}
      , {buffer, 0}
      , size
      );
  }
  PIMPL_DTOR (vmem_allocation)

  std::size_t vmem_allocation::size() const
  {
    return _->_size;
  }
  std::unique_ptr<gpi::pc::client::api_t> const& vmem_allocation::api() const
  {
    return _->_api;
  }

  gpi::pc::type::range_t vmem_allocation::global_memory_range
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

    gpi::pc::type::range_t range (_->_handle_id, offset, size);
    return range;
  }
  gpi::pc::type::range_t vmem_allocation::global_memory_range() const
  {
    return global_memory_range (0UL, _->_size);
  }

  vmem_allocation::vmem_allocation (vmem_allocation&& other)
    : _ (std::move (other._))
  {}
}
