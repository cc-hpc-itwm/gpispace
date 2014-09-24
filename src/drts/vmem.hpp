#ifndef DRTS_VMEM_HPP
#define DRTS_VMEM_HPP

#include <drts/drts.hpp>
#include <drts/rif.hpp>

namespace gspc
{
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
    gspc::rif_t& _rif;
    const std::list<gspc::rif_t::endpoint_t> _rif_endpoints;
  };
}

#endif
