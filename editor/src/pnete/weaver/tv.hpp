// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef FHG_PNETE_WEAVER_TV_HPP
#define FHG_PNETE_WEAVER_TV_HPP

#include <pnete/data/handle/function.fwd.hpp>

class QStandardItem;

namespace fhg
{
  namespace pnete
  {
    namespace weaver
    {
      namespace treeview
      {
        void function (QStandardItem*, const data::handle::function&);
      }
    }
  }
}

#endif
