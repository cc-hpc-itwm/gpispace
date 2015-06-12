#pragma once

#include <drts/virtual_memory.fwd.hpp>
#include <drts/pimpl.hpp>
#include <drts/drts.fwd.hpp>

#include <we/type/value.hpp>

#include <string>

namespace gspc
{
  class scoped_runtime_system;

  class vmem_allocation
  {
  private:
    friend class scoped_runtime_system;

    vmem_allocation ( scoped_runtime_system const* const
                    , unsigned long size
                    , std::string const& description
                    );
    vmem_allocation ( scoped_runtime_system const* const
                    , unsigned long size
                    , std::string const& description
                    , char const* const data
                    );

  public:
    std::string const handle() const;
    std::size_t size() const;

    pnet::type::value::value_type global_memory_range() const;
    pnet::type::value::value_type global_memory_range ( std::size_t const offset
                                                      , std::size_t const size
                                                      ) const;

    vmem_allocation (vmem_allocation const&) = delete;
    vmem_allocation& operator= (vmem_allocation const&) = delete;

    vmem_allocation (vmem_allocation&&);
    vmem_allocation& operator= (vmem_allocation&&) = delete;

    PIMPL (vmem_allocation);
  };
}
