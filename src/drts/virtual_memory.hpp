// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_VMEM_HPP
#define DRTS_VMEM_HPP

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
    ~vmem_allocation();

    std::string const handle() const;

    vmem_allocation (vmem_allocation const&) = delete;
    vmem_allocation& operator= (vmem_allocation const&) = delete;

    vmem_allocation (vmem_allocation&&);
    vmem_allocation& operator= (vmem_allocation&&) = delete;

  private:
    struct implementation;

    implementation* _;
  };
}

#endif
