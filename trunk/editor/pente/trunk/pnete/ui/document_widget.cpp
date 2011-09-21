// bernd.loerwald@itwm.fraunhofer.de

#include <pnete/ui/document_widget.hpp>
#include <pnete/ui/base_editor_widget.hpp>

#include <pnete/util.hpp>

#include <pnete/ui/net_widget.hpp>
#include <pnete/ui/module_call_widget.hpp>
#include <pnete/ui/expression_widget.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      document_widget::document_widget (const data::proxy::type& proxy)
        : dock_widget (data::proxy::name (proxy, "<<anonymous widget>>"))
      {
        connect ( this
                , SIGNAL (visibilityChanged (bool))
                , SLOT (visibility_changed (bool))
                );
      }

      void document_widget::visibility_changed (bool visible)
      {
        if (visible)
        {
          widget()->setFocus();
        }
      }

      base_editor_widget* document_widget::widget() const
      {
        return util::throwing_qobject_cast<base_editor_widget*>
          (dock_widget::widget());
      }

      void document_widget::setWidget (base_editor_widget* widget)
      {
        dock_widget::setWidget (widget);
      }

      net_view::net_view ( data::proxy::type& proxy
                         , data::proxy::net_proxy::data_type& net
                         , graph::scene* scene
                         )
        : document_widget (proxy)
      {
        setWidget (new net_widget (proxy, net, scene, this));

        setWindowTitle (widget()->name());
      }

      expression_view::expression_view
        ( data::proxy::type& proxy
        , data::proxy::expression_proxy::data_type & expression
        )
          : document_widget (proxy)
      {
        setWidget (new expression_widget (proxy, expression, this));

        setWindowTitle (widget()->name());
      }

      mod_view::mod_view
        ( data::proxy::type& proxy
        , data::proxy::mod_proxy::data_type & mod
        )
          : document_widget (proxy)
      {
        setWidget (new module_call_widget (proxy, mod, this));

        setWindowTitle (widget()->name());
      }
    }
  }
}
