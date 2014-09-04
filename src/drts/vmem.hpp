#ifndef DRTS_VMEM_HPP
#define DRTS_VMEM_HPP

#include <drts/drts.hpp>

namespace gspc
{
  class rif_t;

  class vmem_t
  {
  public:
    vmem_t ( boost::program_options::variables_map const& vm
           , gspc::installation const& installation
           , gspc::rif_t& rif
           , std::pair<std::list<std::string>, unsigned long> const&
           , std::string const& kvs_host
           , unsigned short const kvs_port
           );
    ~vmem_t();
  private:
    std::pair<std::list<std::string>, unsigned long> const
    _machinefile;
    gspc::rif_t& _rif;
  };
}

#endif
