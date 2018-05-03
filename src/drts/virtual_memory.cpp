// mirko.rahn@itwm.fraunhofer.de

#include <drts/drts.hpp>
#include <drts/virtual_memory.hpp>

#include <drts/private/drts_impl.hpp>
#include <drts/private/pimpl.hpp>
#include <drts/private/scoped_allocation.hpp>
#include <drts/private/virtual_memory_impl.hpp>

#include <we/type/value/poke.hpp>

#include <boost/format.hpp>

#include <exception>
#include <sstream>

namespace gspc
{
  vmem_allocation::vmem_allocation ( scoped_runtime_system const* const drts
                                   , vmem::segment_description segment_desc
                                   , unsigned long size
                                   , std::string const& description
                                   )
    : _ ( new vmem_allocation::implementation
            (drts->_->_virtual_memory_api, segment_desc, size, description)
        )
  {}
  vmem_allocation::vmem_allocation ( scoped_runtime_system const* const drts
                                   , vmem::segment_description segment_desc
                                   , unsigned long size
                                   , std::string const& description
                                   , char const* const data
                                   )
    : vmem_allocation (drts, segment_desc, size, description)
  {
    scoped_allocation const buffer
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

    {
      // taken from gpi-space/pc/type/handle.hpp
      std::ostringstream oss;

      oss << "0x";
      oss.flags (std::ios::hex);
      oss.width (18);
      oss.fill ('0');
      oss << _->_handle_id;

      pnet::type::value::poke (std::list<std::string> {"handle", "name"}, range, oss.str());
    }
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
