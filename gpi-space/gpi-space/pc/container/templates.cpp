/* explicit instantiations */

#ifndef GPI_SPACE_HEADER_ONLY

#include "connector.ipp"
#include "process.ipp"
#include "manager.hpp"

namespace gpi
{
  namespace pc
  {
    namespace container
    {
      template class connector_t<manager_t>;
      template class process_t<manager_t>;
    }
  }
}

#endif
