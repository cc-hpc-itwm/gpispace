// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/base.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        base::base (change_manager_t& change_manager)
          : _change_manager (change_manager)
        { }

        change_manager_t& base::change_manager() const
        {
          return _change_manager;
        }
      }
    }
  }
}
