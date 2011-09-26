// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/change_manager.hpp>
#include <pnete/data/internal.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      change_manager::change_manager (internal & i)
        : _internal (i)
      {}
    }
  }
}
