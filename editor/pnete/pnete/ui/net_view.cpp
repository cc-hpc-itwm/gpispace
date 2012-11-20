// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/net_view.hpp>

#include <QStringList>
#include <QString>

#include <pnete/ui/net_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      net_view::net_view ( data::proxy::type& proxy
                         , data::proxy::net_proxy::data_type& net
                         , graph::scene_type* scene
                         )
        : document_view (proxy)
      {
        //! \todo submit known types
        setWidget (new net_widget (proxy, net, scene, QStringList(), this));
        set_title (data::proxy::function (proxy).get().name());
      }
      QString net_view::fallback_title () const
      {
        return tr("<<anonymous net>>");
      }
    }
  }
}
