#pragma once

#include <drts/pimpl.hpp>
#include <drts/drts.fwd.hpp>
#include <drts/virtual_memory.fwd.hpp>

#include <we/type/value.hpp>

#include <iml/client/virtual_memory.hpp>

#include <boost/filesystem/path.hpp>

#include <string>
#include <iostream>

namespace gspc
{
  namespace vmem
  {
    using beegfs_segment_description = iml_client::vmem::beegfs_segment_description;
    using gaspi_segment_description = iml_client::vmem::gaspi_segment_description;

    using segment_description = iml_client::vmem::segment_description;
  }

  class vmem_allocation
  {
  private:
    friend class scoped_runtime_system;

    iml_client::vmem_allocation _alloc;

    vmem_allocation (iml_client::vmem_allocation alloc);
    iml_client::vmem_allocation const& iml_allocation() const
    {
      return _alloc;
    }

  public:
    std::size_t size() const;
    pnet::type::value::value_type global_memory_range() const;
    pnet::type::value::value_type global_memory_range ( std::size_t const offset
                                                      , std::size_t const size
                                                      ) const;

    vmem_allocation (vmem_allocation const&) = delete;
    vmem_allocation& operator= (vmem_allocation const&) = delete;

    vmem_allocation (vmem_allocation&&);
    vmem_allocation& operator= (vmem_allocation&&) = delete;
  };
}
