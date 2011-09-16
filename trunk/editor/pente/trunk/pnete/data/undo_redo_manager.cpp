// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/data/undo_redo_manager.hpp>
#include <pnete/data/internal.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      undo_redo_manager::undo_redo_manager (internal & i)
        : _internal (i)
      {}
    }
  }
}
