// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/net_view.hpp>

#include <pnete/ui/graph_view.hpp>

#include <xml/parse/type/function.hpp>

#include <QStringList>
#include <QString>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      net_view::net_view ( data::proxy::type& proxy
                         , const data::handle::function& function
                         , graph::scene_type* scene
                         )
        : document_view (function, proxy, tr ("<<anonymous net>>"))
      {
        setWidget (new graph_view (scene, this));
      }
    }
  }
}
