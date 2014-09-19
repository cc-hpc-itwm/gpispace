// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_VMEM_HPP
#define DRTS_VMEM_HPP

#include <drts/virtual_memory.fwd.hpp>
#include <drts/drts.fwd.hpp>

#include <we/type/value.hpp>

#include <memory>
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

  public:
    //! \note default, but implementation::~implementation() only known in .cpp
    ~vmem_allocation();

    std::string const handle() const;

    pnet::type::value::value_type global_memory_range() const;

    vmem_allocation (vmem_allocation const&) = delete;
    vmem_allocation& operator= (vmem_allocation const&) = delete;

    vmem_allocation (vmem_allocation&&);
    vmem_allocation& operator= (vmem_allocation&&) = delete;

  private:
    struct implementation;

    std::unique_ptr<implementation> _;
  };
}

#endif
