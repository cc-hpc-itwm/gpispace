#include <drts/virtual_memory.hpp>

#include <drts/drts_iml.hpp>

#include <iml/vmem/gaspi/pc/type/types.hpp>

#include <boost/format.hpp>

#include <cstddef>
#include <stdexcept>
#include <utility>

namespace gspc
{
  vmem_allocation::vmem_allocation (iml_client::vmem_allocation alloc)
    : _alloc (std::move (alloc))
  {}
  std::size_t vmem_allocation::size() const
  {
    return _alloc.size();
  }

  ::pnet::type::value::value_type vmem_allocation::global_memory_range
    ( std::size_t const offset
    , std::size_t const size
    ) const
  {
    if ((offset + size) > _alloc.size())
    {
      throw std::logic_error
        ((boost::format ("slice [%1%, %2%) is outside of allocation")
         % offset % (offset + size)
         ).str()
        );
    }

    return pnet::vmem::range_to_value
      (_alloc.global_memory_range (offset, size));
  }
  ::pnet::type::value::value_type vmem_allocation::global_memory_range() const
  {
    return global_memory_range (0UL, _alloc.size());
  }
}
