/* explicit instantiations */

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
