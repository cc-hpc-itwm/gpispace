// {bernd.loerwald,mirko.rahn}@itwm.fraunhofer.de

#include <pnete/ui/document_view.hpp>

#include <QStringList>

#include <pnete/ui/base_editor_widget.hpp>
#include <pnete/data/internal.hpp>

#include <pnete/util.hpp>

namespace fhg
{
  namespace pnete
  {
    namespace ui
    {
      document_view::document_view (const data::proxy::type& proxy)
        : dock_widget ()
      {
        connect ( this
                , SIGNAL (visibilityChanged (bool))
                , SLOT (visibility_changed (bool))
                );

        connect ( &data::proxy::root (proxy)->change_manager()
                , SIGNAL ( signal_set_function_name
                           ( const QObject*
                           , const ::xml::parse::type::function_type&
                           , const QString&
                           )
                         )
                , SLOT ( slot_set_function_name
                         ( const QObject*
                         , const ::xml::parse::type::function_type&
                         , const QString&
                         )
                       )
                );
      }
      void
      document_view::slot_set_function_name
      ( const QObject*
      , const ::xml::parse::type::function_type& function
      , const QString& name
      )
      {
        if (&function != &data::proxy::function (widget()->proxy()))
        {
          return;
        }

        set_title ( name.isEmpty()
                  ? boost::none
                  : boost::optional<std::string> (name.toStdString())
                  );
      }
      void document_view::visibility_changed (bool visible)
      {
        if (visible)
        {
          emit focus_gained (this);
        }
      }
      base_editor_widget* document_view::widget() const
      {
        return util::throwing_qobject_cast<base_editor_widget*>
          (dock_widget::widget());
      }
      void document_view::setWidget (base_editor_widget* widget)
      {
        dock_widget::setWidget (widget);
      }
      void
      document_view::set_title (const boost::optional<std::string>& name)
      {
        setWindowTitle ( name
                       ? QString::fromStdString (*name)
                       : fallback_title()
                       );
      }
    }
  }
}
