// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#ifndef _PNETE_UI_NET_VIEW_HPP
#define _PNETE_UI_NET_VIEW_HPP 1

#include <QObject>

#include <pnete/data/handle/function.fwd.hpp>
#include <pnete/data/proxy.hpp>
#include <pnete/ui/document_view.hpp>
#include <pnete/ui/graph/scene.fwd.hpp>

class QString;

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      class net_view : public document_view
      {
        Q_OBJECT;

      public:
        net_view ( data::proxy::type&
                 , const data::handle::function&
                 , graph::scene_type*
                 );
      };
    }
  }
}

#endif
