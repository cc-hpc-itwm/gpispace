// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/data/handle/module.hpp>

#include <pnete/data/change_manager.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace data
    {
      namespace handle
      {
        module::module ( const module_meta_base::id_type& id
                       , internal_type* document
                       )
          : module_meta_base (id, document)
        { }
      }
    }
  }
}
