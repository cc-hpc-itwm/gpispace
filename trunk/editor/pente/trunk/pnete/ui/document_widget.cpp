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
        : dock_widget ()
      {
        connect ( this
                , SIGNAL (visibilityChanged (bool))
                , SLOT (visibility_changed (bool))
                );

        connect ( &data::proxy::root (proxy)->change_manager()
                , SIGNAL ( signal_set_function_name
                           ( ::xml::parse::type::function_type&
                           , const QString&
                           )
                         )
                , SLOT ( slot_set_function_name
                         ( ::xml::parse::type::function_type&
                         , const QString&
                         )
                       )
                );
      }
      void
      document_widget::slot_set_function_name
      ( ::xml::parse::type::function_type& function
      , const QString& name
      )
      {
        if (&function != &data::proxy::function (widget()->proxy()))
        {
          return;
        }

        set_title ( name.isEmpty()
                  ? fhg::util::Nothing<std::string>()
                  : fhg::util::Just<std::string> (name.toStdString())
                  );
      }
      void document_widget::visibility_changed (bool visible)
      {
        if (visible)
        {
          emit focus_gained (this);
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
      void
      document_widget::set_title (const fhg::util::maybe<std::string>& name)
      {
        setWindowTitle (name ? QString((*name).c_str()) : fallback_title());
      }

      net_view::net_view ( data::proxy::type& proxy
                         , data::proxy::net_proxy::data_type& net
                         , graph::scene* scene
                         )
        : document_widget (proxy)
      {
        //! \todo submit known types
        setWidget (new net_widget (proxy, net, scene, QStringList(), this));
        set_title (data::proxy::function (proxy).name);
      }
      QString net_view::fallback_title () const
      {
        return tr("<<anonymous net>>");
      }

      expression_view::expression_view
        ( data::proxy::type& proxy
        , data::proxy::expression_proxy::data_type & expression
        )
          : document_widget (proxy)
      {
        //! \todo submit known types
        setWidget (new expression_widget ( proxy
                                         , expression
                                         , QStringList()
                                         , this
                                         )
                  );
        set_title (data::proxy::function (proxy).name);
      }
      QString expression_view::fallback_title () const
      {
        return tr("<<anonymous expression>>");
      }

      mod_view::mod_view
        ( data::proxy::type& proxy
        , data::proxy::mod_proxy::data_type & mod
        )
          : document_widget (proxy)
      {
        //! \todo submit known types
        setWidget (new module_call_widget (proxy, mod, QStringList(), this));
        set_title (data::proxy::function (proxy).name);
      }
      QString mod_view::fallback_title () const
      {
        return tr("<<anonymous module call>>");
      }
    }
  }
}
