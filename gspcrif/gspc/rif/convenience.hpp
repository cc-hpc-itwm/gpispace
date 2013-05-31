#ifndef GSPC_RIF_CONVENIENCE_HPP
#define GSPC_RIF_CONVENIENCE_HPP

#include <gspc/rif/types.hpp>
#include <gspc/rif/manager_fwd.hpp>

namespace gspc
{
  namespace rif
  {
    proc_t exec (manager_t & mgr, std::string const &cmdline);
  }
}

#endif
