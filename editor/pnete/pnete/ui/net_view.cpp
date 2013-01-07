// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/net_view.hpp>

#include <pnete/ui/net_widget.hpp>

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
                         , graph::scene_type* scene
                         )
        : document_view (data::proxy::function (proxy), proxy)
      {
        //! \todo submit known types
        setWidget (new net_widget (proxy, scene, this));
        set_title (data::proxy::function (proxy).get().name());
      }
      QString net_view::fallback_title () const
      {
        return tr("<<anonymous net>>");
      }
    }
  }
}
