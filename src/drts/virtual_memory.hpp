// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_VMEM_HPP
#define DRTS_VMEM_HPP

#include <boost/filesystem.hpp>

#include <string>

namespace gspc
{
  class vmem_allocation
  {
  private:
    friend class scoped_runtime_system;

    vmem_allocation ( boost::filesystem::path const& gspc_home
                    , boost::filesystem::path const& vmem_socket
                    , unsigned long size
                    , std::string const& description
                    );

  public:
    ~vmem_allocation();

    std::string const& handle() const
    {
      return _handle;
    }

    vmem_allocation (vmem_allocation const&) = delete;
    vmem_allocation& operator= (vmem_allocation const&) = delete;

    vmem_allocation (vmem_allocation&&);
    vmem_allocation& operator= (vmem_allocation&&) = delete;

  private:
    boost::filesystem::path _gspc_home;
    boost::filesystem::path _vmem_socket;
    std::string _handle;
    bool _disowned;
  };
}

#endif
