// mirko.rahn@itwm.fraunhofer.de

#ifndef DRTS_VMEM_HPP
#define DRTS_VMEM_HPP

#include <boost/filesystem.hpp>

#include <string>

namespace gspc
{
  class vmem_allocation
  {
  public:
    vmem_allocation ( boost::filesystem::path const& gspc_home
                    , boost::filesystem::path const& vmem_socket
                    , unsigned long size
                    , std::string const& description
                    );
    ~vmem_allocation();
    std::string const& handle() const;

  private:
    boost::filesystem::path const _gspc_home;
    boost::filesystem::path const _vmem_socket;
    std::string const _handle;
  };
}

#endif
